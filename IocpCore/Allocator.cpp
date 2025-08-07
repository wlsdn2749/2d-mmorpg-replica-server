#include "pch.h"
#include "Allocator.h"
#include "Memory.h"

/*----------------------
 * BaseAllocator
 ----------------------*/

void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}

/*----------------------
 * StompAllocator
 ----------------------*/

// 이걸 쓰는 이유가 StompAllocator는 Use-After-Free를 잡아줄 수 있음

void* StompAllocator::Alloc(int32 size)
{
	// 설령 4096이여도. 2개가 아니라 1.999개로 딱떨어짐 
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	const int64 dataOffset = pageCount * PAGE_SIZE - size;
	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
	// 뒷부분에 위치하도록 void* -> int8*로 바꾸고 (byte)
	// byte연산을 통해 dataOffset만큼 움직이고,
	// 다시 void*로 바꾸고 리턴


	// 매개변수
	// 1. IpAddress: Null로하면 알아서 잡아줌
	// 2. dwSize: 할당할 메모리 크기
	// 3. /// : 메모리 옵션
	// 4. 정책 : 그 영역을 어떻게 할것인지 R, W, RW, X

}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}



/*----------------------
 * PoolAllocator
 ----------------------*/


void* PoolAllocator::Alloc(int32 size)
{
	return GMemory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}
