#include "pch.h"
#include "CoreTLS.h"

thread_local uint32 LThreadId = 0;
thread_local std::stack<int32>	LLockStack;
thread_local SendBufferChunkRef	LSendBufferChunk;
thread_local JobQueue*			LCurrentJobQueue = nullptr;
thread_local uint64				LEndTickCount = 0;