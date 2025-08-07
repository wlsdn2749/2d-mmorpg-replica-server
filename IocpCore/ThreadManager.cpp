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

void ThreadManager::DoGlobalQueueWork()
{
	while (true)
	{
		uint64 now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;

		JobQueueRef jobQueue = GGlobalQueue->Pop();
		if (jobQueue == nullptr)
			break;

		jobQueue->Execute();
	}
}

