#pragma once

#include <type_traits>
/*----------------------
 * BaseAllocator
 ----------------------*/

class BaseAllocator
{
public:
	static void*	Alloc(int32 size);
	static void		Release(void* ptr);
};

/*----------------------
 * StompAllocator
 ----------------------*/


class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 };
public:
	static void*	Alloc(int32 size);
	static void		Release(void* ptr);

};


/*----------------------
 * PoolAllocator
 ----------------------*/


class PoolAllocator
{

public:
	static void*	Alloc(int32 size);
	static void		Release(void* ptr);

};

/*----------------------
 * STLAllocator
 ----------------------*/
template <typename T>
class StlAllocator
{
public:
	using value_type = T;

	// --- MSVC가 최적화/검사에 쓰는 traits ---
	using propagate_on_container_swap = std::true_type;
	using propagate_on_container_move_assignment = std::true_type;
	using is_always_equal = std::true_type;

	StlAllocator() {}

	template <typename Other>
	StlAllocator(const StlAllocator<Other>&) {}

	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(PoolAllocator::Alloc(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		PoolAllocator::Release(ptr);
	}

	
};

// --- 동등 비교 연산자: 타입 달라도 true/false 결정 가능하도록 템플릿으로 제공 ---
template <class T, class U>
constexpr bool operator==(const StlAllocator<T>&, const StlAllocator<U>&) noexcept { return true; }

template <class T, class U>
constexpr bool operator!=(const StlAllocator<T>&, const StlAllocator<U>&) noexcept { return false; }