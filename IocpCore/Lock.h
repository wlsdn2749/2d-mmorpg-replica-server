#pragma once
#include "Types.h"

/*----------------
	RW SpinLock
-----------------*/

/*---------------------------------------------- 
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR] 
W: WriteFlag (Exclusive Lock Owner ThreadId)
R: ReadFlag (Shared Lock Count)
-----------------------------------------------*/

// Write -> Read (O)
// Write -> Write (O)
// Read -> Write (X)
// Read -> Read (O)

class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000, // 1만틱
		MAX_SPIN_COUNT = 5000, // 최대로 SPIN하는 Count
		WRITE_THREAD_MASK = 0xFFFF'0000, // WRITE_THREAD의 정보를 얻어오는 MASK
		READ_COUNT_MASK = 0x0000'FFFF, // READ_THREAD의 정보를 얻어오는 MASK
		EMPTY_FLAG = 0x0000'0000 // 아무것도 없음
	};
public:
	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);
private:
	Atomic<uint32> _lockFlag;
	uint16 _writeCount = 0;

};

/*-----------------
 *	LockGuards
 *---------------*/

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.ReadLock(name); }
	~ReadLockGuard(){_lock.ReadUnlock(_name);}
private:
	Lock& _lock;
	const char* _name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.WriteLock(name); }
	~WriteLockGuard() { _lock.WriteUnlock(_name); }

private:
	Lock& _lock;
	const char* _name;
};