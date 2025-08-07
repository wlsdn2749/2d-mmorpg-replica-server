// 스레드별로 가지고 있는 TLS(Thread Local Storage) 영역에 대한 선언을 작성하는 곳
#pragma once
#include <stack>

extern thread_local uint32				LThreadId;
extern thread_local uint64				LEndTickCount;

extern thread_local std::stack<int32>	LLockStack;
extern thread_local SendBufferChunkRef	LSendBufferChunk;
extern thread_local class JobQueue*		LCurrentJobQueue;
