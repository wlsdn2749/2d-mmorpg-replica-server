#pragma once
#include "Types.h"

#pragma region TypeList
template<typename... T>
struct TypeList;

template<typename T, typename U>
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

template<typename T, typename... U>
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};

#pragma endregion


#pragma region Length
template<typename T>
struct Length;

template<>
struct Length<TypeList<>>
{
	enum {value = 0}; // enum도 컴파일 타임에 완료됨
};

template<typename T, typename... U>
struct Length<TypeList<T, U...>>
{
	enum {value = 1 + Length<TypeList<U...>>::value };
};

#pragma endregion

#pragma region TypeAt
template<typename TL, int32 index>
struct TypeAt;

template<typename Head, typename... Tail>
struct TypeAt<TypeList<Head, Tail...>, 0>
{
	using Result = Head;
};

template<typename Head, typename... Tail, int32 index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};

#pragma endregion

#pragma region IndexOf
template<typename TL, typename T>
struct IndexOf;

template<typename... Tail, typename T>
struct IndexOf<TypeList<T, Tail...>, T>
{
	enum { value = 0 };
};

template<typename T> 
struct IndexOf<TypeList<>, T> 
{
	enum {value = -1}; // 못 찾은 경우
};

template<typename Head, typename... Tail, typename T>
struct IndexOf<TypeList<Head, Tail...>, T>
{
private:
	enum {temp = IndexOf<TypeList<Tail...>, T>::value};
public:
	enum {value = (temp == -1) ? -1 : temp+1};
};

#pragma endregion



#pragma region Conversion
template<typename From, typename To>
class Conversion
{
private:
	using Small = __int8;
	using Big = __int32;

	static Small Test(const To&) {return 0;}
	static Big Test(...) {return 0;}
	static From MakeFrom() {return 0;}
public:
	enum
	{
		exists = sizeof(Test(MakeFrom())) == sizeof(Small)

		// MakeFrom이 반환하는 형태가 From인데
		// From이 is-a To 형태로 To가 From을 상속받고 있다면?
		// Test(const To&)가 호출이되서 Small이 나오고
		// 그 size는 Small이므로 exist 가 True
		// is-a 형태가 아니면 Test(...)으로 들어가서
		// Big이 되므로 False?
	};
};
#pragma endregion


#pragma region TypeCast

template<int32 v>
struct int2Type
{
	enum
	{
		value = v
	};
};


template<typename TL>
class TypeConversion
{
public:
	enum
	{
		length = Length<TL>::value
	};

	TypeConversion()
	{
		MakeTable(int2Type<0>(), int2Type<0>());
	}

	template<int32 i, int32 j>
	static void MakeTable(int2Type<i>, int2Type<j>)
	{

		using FromType = typename TypeAt<TL, i>::Result;
		using ToType = typename TypeAt<TL, j>::Result;

		if (Conversion<const FromType*, const ToType*>::exists)
		{
			s_convert[i][j] = true;
		}
		else
		{
			s_convert[i][j] = false;
		}

		MakeTable(int2Type<i>(), int2Type<j+1>());

	}

	template<int32 i>
	static void MakeTable(int2Type<i>, int2Type<length>)
	{
		MakeTable(int2Type<i+1>(), int2Type<0>());
	}

	template<int j>
	static void MakeTable(int2Type<length>, int2Type<j>)
	{
		
	}

	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion conversion;
		return s_convert[from][to];
	}
public:
	static bool s_convert[length][length];
};

template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];

template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
shared_ptr<To> TypeCast(shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_pointer_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;

	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);

}


template<typename To, typename From>
bool CanCast(shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;

	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);

}

#pragma endregion


#define DECLARE_TL		using TL = TL; int _typeId;
#define INIT_TL(Type)	_typeId = IndexOf<TL, Type>::value;