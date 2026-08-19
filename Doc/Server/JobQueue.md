# 1. JobQueue

필드에 존재하는 Object들의 처리를 여러 스레드가 동시에 처리할 수 있다면
Data race가 발생할 수 있어 Stateful한 GameServer에서는 치명적일 수 있고
이로 인한 락 경합, 이로 인한 지연, 결국 싱글스레드로 처리하는 것보다 Throughput이 낮아질 수 있고 
코드를 짜는 것에 있어서도 매번 멀티스레드를 신경써야하기 때문에 매우 불편함.


## 2. JobQueue와 JobTimer
기본적으로 JobQueue의 Queue는 `ConcurrentQueue`의 구조를 따라감.
Network I/O Thread에서 접근 할 수 있기 때문에 내부적으로 구현한 `LockQueue`로 Push, Pop할때 Lock걸고 사용
특정 Room에서 실행하는 `std::function<void(void)>` 형태의 Job을 만들어 Push까지 진행

각 Room이 소유하고 있는 JobQueue에는, 2가지 인터페이스가 있음
1. DoAsync
```cpp
template<typename F>
void DoAsync(F&& f)
{
    Push(ObjectPool<Job>::MakeShared(std::forward<F>(f)));
}

template<typename T, typename Ret, typename... FArgs, typename... CallArgs>
DoAsync(Ret(T::* memFunc)(FArgs...), CallArgs&&... callArgs)
{
    std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());

    auto task = [owner = std::move(owner), memFunc,
        tup = std::make_tuple(std::forward<CallArgs>(callArgs)...)]() mutable
        {
            std::apply([&](auto&&... args) {
                std::invoke(memFunc, owner.get(), std::forward<decltype(args)>(args)...);
                }, tup);
        };

    DoAsync(std::move(task)); // 위 콜러블 DoAsync로 위임
}
```
`Room->DoAsync(&Room::HandleMethod)` 이런 형태로 호출되는 시그니처에는

- Room::HandleMethod의 함수 시그니처를 그대로 가져오고, 매개변수까지 그대로 넘김
- std::apply로 함수에 매개변수를 전달하고, std::invoke로 (*owner).*memFunc(CallArgs) 형태로 실행하도록 Job을 구성


```cpp
Job(CallbackType&& callback) : _callback(std::move(callback))
{
    
}

void Execute()
{
    _callback();
    //std::cout << "Job : 실제 잡 실행" << std::endl;
}
```

Job의 생성자는 이렇게 되어있기 때문에 `std::function<void(void)>` 형인 _callback이 할당되고 나중에 Execute에서 실제 실행 함.

2. DoTimer
```cpp
template<typename F>
void DoTimer(uint64 tickAfter, F&& f)
{
    JobRef job = ObjectPool<Job>::MakeShared(std::forward<F>(f));
    GJobTimer->Reserve(tickAfter, shared_from_this(), job);
}

// non-const 멤버 함수 버전
template<typename T, typename Ret, typename... FArgs, typename... CallArgs>
void DoTimer(uint64 tickAfter, Ret(T::* memFunc)(FArgs...), CallArgs&&... callArgs)
{
    std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
    auto task = [owner = std::move(owner), memFunc,
        tup = std::make_tuple(std::forward<CallArgs>(callArgs)...)]() mutable
        {
            std::apply([&](auto&&... args) {
                std::invoke(memFunc, owner.get(), std::forward<decltype(args)>(args)...);
                }, tup);
        };
    DoTimer(tickAfter, std::move(task)); // 첫 번째 오버로드로 위임
}
```

기존 DoAsync와 거의비슷한데 추가적으로 몇ms 후에 시작할지를 정하는 tickAfter 인자가 추가된다.
이렇게 만들어진 Job객체는 전역타이머에 저장되는데
실행될 `(Room)JobQueue Owner`정보와, 실행할 시간을 담은 `executeTick`을 `TimerItem Struct`에 추가하고
전역 우선순위 큐인 `_items`에 넣게 된다.

```cpp
void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job)
{
    const uint64 executeTick = ::GetTickCount64() + tickAfter;
    JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

    WRITE_LOCK;
    _items.push(TimerItem { executeTick, jobData });
}
```

그럼 이렇게 GlobalTimerQueue와 각 JobQueue에 쌓인 TimerJob과 Job들은 어떻게 처리되는가?

## 3. JobTimer와 Job의 처리

```cpp
GThreadManager->Launch([q, timeBudgetMs] {
    while (true) {
        // 타이머 만료 분배
        GThreadManager->DistributeReservedJobs();
        // 이번 사이클 예산 설정
        LEndTickCount = ::GetTickCount64() + timeBudgetMs;
        // 이 큐만 처리
        GThreadManager->DoGlobalQueueWork(q.get());
    }
    });
```

처음 Room을 Sharding할 때, 각 Room이 각 틱에 어떤 일을 하도록 스레드를 할당한다.
이때 실행하는 프로시저로, 타이머 만료 분배, 그리고 이 큐만 처리하는 로직이 있다.


```cpp
void ThreadManager::DistributeReservedJobs()
{
    const uint64 now = ::GetTickCount64();

    // 현재 시간으로 JobTimer의 우선순위 큐에서 뽑을 Job들을 선별
    GJobTimer->Distribute(now);
}

void JobTimer::Distribute(uint64 now)
{
	// 한 번에 1 스레드만 통과

	if (_distributing.exchange(true) == true)
		return; // 이전값이 true이면 였으면 그냥 리턴, 아니면 통과

    // 만료된 Timer들을 items에 넣음
	Vector<TimerItem> items;
	{
		WRITE_LOCK;

		while (_items.empty() == false)
		{
			const TimerItem& timerItem = _items.top();
			if (now < timerItem.executeTick)
				break;

			items.push_back(timerItem);
			_items.pop();
			
		}
	}

    // 각 Timer들을 실제 JobQueue에 Push해줌. (이때부터는 Job과 동일하게 적용)
	for (TimerItem& item : items)
	{
		if (JobQueueRef owner = item.jobData->owner.lock()) // item의 소유자를 체크해서 JobQueueRef에 넣고, 그걸 nullptr체크까지
		{
			owner->Push(item.jobData->job);
		}
		ObjectPool<JobData>::Push(item.jobData);
	}


	// 끝났으면 풀어준다.
	_distributing.store(false);
}


```
이렇게 특정 한 스레드가 글로벌 타이머에 대한 분배를 한다.
글로벌 타이머에 대한 분배를 따로 처리하는 스레드나, 각 JobQueue마다 타이머가 존재하는게 아니라
일반 JobQueue Thread가 글로벌 타이머에 대한 분배를 실행하는 구조이다.


이후 DoGlobalQueueWork에서

```cpp
void ThreadManager::DoGlobalQueueWork(GlobalQueue* q)
{
	while (true)
	{
		// 1) 큐에서 JobQueue 하나 블로킹 Pop
		JobQueueRef jq = q->Pop();
		if (!jq) return;               // 종료 시엔 nullptr를 넣어 깨우는 방식 사용

		// 2) 이번 사이클 예산 설정은 바깥(부팅 루프)에서
		jq->Execute();

		// 3) (선택) 예산 소진 시 바깥 루프로 돌아가 새 예산 설정
		if (::GetTickCount64() > LEndTickCount)
			return;
	}
}

void JobQueue::Execute()
{
    LCurrentJobQueue = this;

    while (true)
    {
        Vector<JobRef> jobs;
        _jobs.PopAll(OUT jobs);

        const int32 jobCount = static_cast<int32>(jobs.size());

        for (int32 i = 0; i < jobCount; i++)
        {
            jobs[i]->Execute(); // 잡 실행

        }

        // 남은 일감 0개라면 종료
        if (_jobCount.fetch_sub(jobCount) == jobCount) // 카운트 증감
        {
            LCurrentJobQueue = nullptr;
            return;	
        }

        const uint64 now = ::GetTickCount64();
        if (now >= LEndTickCount)
        {
            LCurrentJobQueue = nullptr;

            if(_owner) _owner->Push(shared_from_this());
            break;
        }
    }
}
```
각 스레드는 자신의 jobQueue 큐를 돌면서 할당된 맵들이 밀어넣은 job들을 처리한다.











