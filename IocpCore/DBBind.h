#pragma once
#include "DBConnection.h"

/*---------------------
 Template for Validate
 ---------------------*/
template<int32 C>
struct FullBits{enum {value = ( 1 << (C-1)) | FullBits<C-1>::value }; };

template<>
struct FullBits<1>{enum {value = 1};};

template<>
struct FullBits<0>{enum { value = 0 }; };

// FullBits<3> = (1 << 2) | (FullBits<2> == (1 << 1)) | (FullBits<1> == 1)
// (1 << 2) | (1 << 1) | 1
// value = b'0x111'

/*------------
	DBBind
 ------------*/

template<int32 ParamCount, int32 ColumnCount>
class DBBind
{
public:
	DBBind(DBConnection& dbConnection, const WCHAR* query)
		: _dbConnection(dbConnection), _query(query)
	{
		::memset(_paramIndex, 0, sizeof(_paramIndex));
		::memset(_columnIndex, 0, sizeof(_columnIndex));
		_paramFlag = 0;
		_columnFlag = 0;
		_hasOutputParams = false;
		dbConnection.Unbind();
	}

	bool Validate()
	{
		return _paramFlag == FullBits<ParamCount>::value && _columnFlag == FullBits<ColumnCount>::value;
	}

	bool Execute()
	{
		ASSERT_CRASH(Validate());
		return _dbConnection.Execute(_query, _hasOutputParams);
	}

	bool Fetch()
	{
		return _dbConnection.Fetch();
	}

public:
	template<typename T> // 이 템플릿으로 대부분 처리되지만
	void BindParam(int32 idx, T& value, SQLSMALLINT direction = SQL_PARAM_INPUT)
	{
		if (direction == SQL_PARAM_OUTPUT)
		{
			_hasOutputParams = true;
		}
		_dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx], direction); // idx는 0번부터 시작 -> 실제로는 1번이라 1더함
		_paramFlag |= (1LL << idx); // idx위치에 있는 데이터의 비트플래그를 1로 켜줌
	}

	void BindParam(int32 idx, const WCHAR* value, SQLSMALLINT direction = SQL_PARAM_INPUT) // 문자열 
	{
		if (direction == SQL_PARAM_OUTPUT)
		{
			_hasOutputParams = true;
		}
		_dbConnection.BindParam(idx + 1, value, &_paramIndex[idx], direction);
		_paramFlag |= (1LL << idx);
	}

	template<typename T, int32 N> // 바이트 배열
	void BindParam(int32 idx, T(&value)[N], SQLSMALLINT direction = SQL_PARAM_INPUT)
	{
		if (direction == SQL_PARAM_OUTPUT)
		{
			_hasOutputParams = true;
		}
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N, &_paramIndex[idx], direction);
		_paramFlag |= (1LL << idx);
	}

	template<typename T> // 바이트 배열의 개수
	void BindParam(int32 idx, T* value, int32 N, SQLSMALLINT direction = SQL_PARAM_INPUT)
	{
		if (direction == SQL_PARAM_OUTPUT)
		{
			_hasOutputParams = true;
		}
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N, &_paramIndex[idx], direction);
		_paramFlag |= (1LL << idx); 
	}

	template<typename T>
	void BindCol(int32 idx, T& value)
	{
		_dbConnection.BindCol(idx + 1, &value, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	template<int32 N>
	void BindCol(int32 idx, WCHAR(&value)[N]) // 배열
	{
		_dbConnection.BindCol(idx + 1, value, N-1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	void BindCol(int32 idx, WCHAR* value, int32 len) // 포인터랑 길이
	{
		_dbConnection.BindCol(idx + 1, value, len - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	template<typename T, int32 N>
	void BindCol(int32 idx, T(&value)[N])
	{
		_dbConnection.BindCol(idx + 1, value, size32(T) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	// Array 용 Overload
	template<typename T, size_t N>
	void BindCol(int32 idx, std::array<T, N>& value)
	{
		// value.data() -> 버퍼 첫 주소
		// sizeof(T) * N -> 바이트 수

		_dbConnection.BindCol(idx + 1, value.data(), size32(T) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}


protected:
	DBConnection&	_dbConnection;
	const WCHAR*	_query;
	SQLLEN			_paramIndex[ParamCount > 0 ? ParamCount : 1]; // 컴파일 타임에 결정되는 거라 가능
	SQLLEN			_columnIndex[ColumnCount > 0 ? ColumnCount : 1];
	uint64			_paramFlag; // uint64니까, 데이터를 64개까지 처리가능
	uint64			_columnFlag;
	bool			_hasOutputParams;
};

