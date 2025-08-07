#pragma once
#include "Allocator.h"

class MemoryPool;


/*------------------
 *		Memory
 -----------------*/

class Memory
{
	enum
	{
		// ~1024까지 32단위, ~2048까지 128단위, ~4096까지 256단위

		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096,
	};
public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	vector<MemoryPool*> _pools;

	// 메모리 크기  <--> 메모리 풀
	// O(1)만에 찾기 위한 Helper Table
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};

template<typename Type, typename... Args> // valid template 인자가 여러개 가능
Type* xnew(Args&& ...args) // 가변 길이 인자
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));

	// placement new - 메모리 위에다가 생성자 호출!
	new(memory)Type(std::forward<Args>(args)...); // Args의 완벽한 전달

	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type(); // obj의 소멸자 호출
	PoolAllocator::Release(obj);
}

template<typename Type, typename... Args> // valid template 인자가 여러개 가능
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type> { xnew<Type>(std::forward<Args>(args)...), xdelete<Type> };
}