#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{

#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	// 동일한 스레드가 이미 소유하고 있다면 무조건 성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) // WriteLock을 호출한 스레드가. 이미 락을 잡고있다면
	{
		_writeCount++;
		return;
	}
	// 아무도 소유 및 공유하고 있지 않을때, 경합해서 소유권을 얻는다.
	// 스핀방식으로 뺑뻉이, CAS의 방식 사용 
	// 경합할때는 그냥 못씀
	
	const uint32 desired	= ((LThreadId << 16) & WRITE_THREAD_MASK);
	const int64 beginTick	= ::GetTickCount64();
	//uint32 expected = EMPTY_FLAG; // 아무도 읽거나 쓰지 않을때
	while (true) 
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = EMPTY_FLAG; // 아무도 읽거나 쓰지 않을때
			// expected가 외부에 선언되면 CAS를 하는 과정에서
			// expected가 현재 _lockFlag로 복사되어 들어가므로, 절대로 소유권을 획득할 수 없게됨.
			// 하지만 예외로 READ 중일 때 우현히 expected값과 lockFlag값이 같아질 수 있는데
			// 이때 _lockFlag값을 desired로 변경하게 되고, READ FLAG쪽을 0으로 밀어버리니까
			// READ에서 UNLOCK을 할때 문제가 됨!
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				// 여기서 WriteLock을 획득한 상태
				++_writeCount;
				return;
			}
		}


		// 시간초과 
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT")

		// 최대 스핀 초과 -> yield하고 다시 돌아오도록
		// 이떄 yield는 다시 돌아올 수도 있고, 다른 Thread가 필요한 곳에 할당 될 수도 있음.
		this_thread::yield();
	}

}

void Lock::WriteUnlock(const char* name)
{

#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	// ReadLock을 다 풀기 전에는 WriteUnlock은 불가능
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("INVAILD_UNLOCK_ORDER")

	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
	{
		_lockFlag.store(EMPTY_FLAG);
	}

}

void Lock::ReadLock(const char* name)
{

#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 동일한 스레드가 WriteLock을 잡은 상태에서 ReadLock을 잡는 건 허락
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) // ReadLock을 호출한 스레드가. 이미 락을 잡고 있다면 허락
	{
		_lockFlag.fetch_add(1); // 이게 가능한 이유는 다른 스레드가 접근 불가하니까 가능
		return;
	}


	// 아무도 소유하고 있지 않으면 경합해서 카운트 올림
	const int64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			const uint32 desired = expected + 1;
			// 실패하는 경우
			// 1. expected가 CAS 연산 사이에 변경되는 경우
			// 2. 다른 스레드가 WriteLock을 잡혀있어서. expected값이 _lockFlag값이랑 같을 수가 없음.
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1)) // WRITE를 아무도 잡고 있지 않을때
			{
				return;
			}
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT")

		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{

#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	// fetch_sub(1)으로 가능
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0) // 잘못된 UNLOCK
	{
		CRASH("MULTIPLE_UNLOCK")
	}
}
