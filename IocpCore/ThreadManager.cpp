#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "JobQueue.h"
#include "GlobalQueue.h"

ThreadManager::ThreadManager()
{
	// MainThread 
	initTLS();// 
}

ThreadManager::~ThreadManager()
{
	Join();
}

// std::function<void()>은 온갖 함수를 다 받아줌
// lambda 함수를 포함
// input = void, output = void
void ThreadManager::Launch(std::function<void(void)> callback)
{
	LockGuard guard(_lock);

	_threads.push_back(std::thread([=]()
		{
			initTLS();			// TLS를 생성
			callback();			// callback()함수 실행
			DestroyTLS();		// TLS를 삭제
		}));
}

void ThreadManager::Join()
{
	for (std::thread& t: _threads)
	{
		if (t.joinable()) // 스레드가 끝날 수 있으면
		{
			t.join(); // join으로 종료
		}
	}

	_threads.clear(); // 모든 스레드를 clear함
}

void ThreadManager::initTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
	// 동적으로 생성되는게 있으면 날려줌.
}

void ThreadManager::DistributeReservedJobs()
{
	const uint64 now = ::GetTickCount64();

	GJobTimer->Distribute(now);
}

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
