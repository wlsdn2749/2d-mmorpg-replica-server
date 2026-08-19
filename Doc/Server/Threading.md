# Threading 

게임이 바람의 나라 모작이다 보니까, Map(Room) 별로 스레드를 격리 했을때 Throughput이 높아질 수 있을 것 이라고 생각함.

중요한건. Logic이 돌아가는 부분과, 플레이어 단위의 싱글스레드의 보장이 확실해야 한다고 생각
[JobQueue.md]에서 싱글스레드의 보장은 정리하고, 여기는 스레딩 부분만 서술함

스레드를 3개의 성격으로 분리해서 관리하도록 설계

1. 네트워크 I/O 스레드 (IOCP worker)
   GThreadManager를 통해 DoWorkerJob을 실행하도록 IOCP Worker Thread를 5개 구성
   Worker Thread를 줄이기도, 늘리기도 해봤지만, I/O에서 병목은 없었으므로 Heuristic하게 5개를 구성. -- 측정도구 첨부

   ```C++
    void DoWorkerJob(ServerServiceRef& service)
    {
        while (true)
        {
            LEndTickCount = ::GetTickCount64() + WORKER_TICK;

            // 네트워크 입출력 처리
            service->GetIocpCore()->Dispatch(10);
        }
    }

    bool IocpCore::Dispatch(uint32 timeousMs)
    {
        // IocpEvent가 IocpObject(Session) 객체를 들고 있어, Key를 사용하지 않고
        // I/O시 IocpObjectRef로 Session이 사라지지 않음을 보장함
        if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes,
            /*Key*/OUT &key,
            /*Overlapped*/OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), 
            timeousMs))
        {
            IocpObjectRef iocpObject = iocpEvent->owner;
            iocpObject->Dispatch(iocpEvent, numOfBytes);
        }
        else
        {
            //** Timeout 처리

            // 정상 종료 (recv:0)
            IocpObjectRef iocpObject = iocpEvent->owner;
            iocpObject->Dispatch(iocpEvent, numOfBytes);             
        }

        return true;
    }
    ```

2. DB Thread 
   DB Thread 역시, DB처리는 게임로직과는 별개이므로 분리할 수 있고. 구현의 편의성을 위해 싱글스레드로 시작.
   
    ```cpp
   	void Start() 
    {
	    GThreadManager->Launch([]()
	    {
	        DbThreadMain();
	    });
	}

    static void DbThreadMain()
    {
        DBConnGuard threadConn;
        auto& conn = threadConn.ref();

        Vector<DbJob> batch;
        
        while (true) 
        {
            if (GDbQueue.pop_all(batch) == 0) // 종료조건
                // batch가 비어있을 경우 종료 시퀀스..
            
            if (batch.size() > MAX_BATCH) // 초과분 그냥 삭제 (DB 실패)
                batch.resize(MAX_BATCH);

            for (auto& job : batch)
            {
                try 
                {
                    job(conn); 
                }
                // catch (const std::exception& e)
            }

            batch.clear();
        }
    }


    // 실행은 DB Thread에서, 결과는 future를 get하는 호출 스레드에서
    struct DbDispatcher {
        static std::future<void> Enqueue(std::function<void(DBConnection&)> fn)
        {
            auto taskPtr = MakeShared<std::packaged_task<void(DBConnection&)>>(std::move(fn));
            auto fut = taskPtr->get_future();
            GDbQueue.push([taskPtr](DBConnection& c) { (*taskPtr)(c); });
            return fut;
        }

        template<typename Fn>
        static auto EnqueueRet(Fn fn)
            -> std::future<decltype(fn(std::declval<DBConnection&>()))> // template의 반환타입을 future의 반환타팁으로 매핑
        {
            using R = decltype(fn(std::declval<DBConnection&>()));
            auto taskPtr = MakeShared<std::packaged_task<R(DBConnection&)>>(std::move(fn));
            auto fut = taskPtr->get_future();
            GDbQueue.push([taskPtr](DBConnection &c) mutable {
                (*taskPtr)(c);
            });
            return fut;
        }
    };


    ```

3. Main Thread (Room별 스레드 격리)
   기존 Room, Thread의 1:1구조가 아니라
   바람의 나라 맵 구조상, 한 맵에 여러 사람이 들어가 서버가 처리를 하더라도 자원이 남을것이라고 판단하여      
   Thread 1개가 여러 맵을 처리할 수 있도록 1:N Rooms 구조로 설계하였음.
   
    ```cpp
    // 설정
    constexpr int totalRooms = 3; // 현재 총 Room
    constexpr int roomPersQueue = 2; // 한 JobQueue가 담당하는 Room 개수
    constexpr UINT32 budgetMs = 30; // 한 워커의 수행시간 Slice 
    StartShardedQueues(totalRooms, roomPersQueue, budgetMs);

    // 스레드 생성
    for (int i = 0; i < numQueues; ++i) {
        auto q = make_shared<GlobalQueue>();
        gQueues.push_back(q);

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
    }

    // 각 Map 설정할때, JobQueue 스레드 할당
    void CreateRooms()
    {
        // 마을(1)
        {
            auto town0Map = MapData::LoadMapFromJsonFile("./Resources/Maps/Goguryeo_Tilemap_Info.json");
            auto town0 = std::make_shared<TownRoom>(Room::Cfg{ 1, "Goguryeo", 50, 200 }, town0Map);
            town0->SetOwner(gQueues[OwnerIndexForRoom(town0->RoomId(), roomsPerQueue)].get());
            town0->Init(); // 이 이후 시점부터 Tick
            RoomManager::Instance().Add(town0);
        } 

        // 마을(2) ...
    }
    ```

    

   
   