#include "pch.h"
#include "DBConnection.h"

void PrintOdbcError(SQLSMALLINT handleType, SQLHANDLE handle)
{
	SQLINTEGER   nativeError;
	SQLWCHAR     sqlState[7] = { 0 };
	SQLWCHAR     messageText[256] = { 0 };
	SQLSMALLINT  textLength;
	SQLRETURN    ret;
	SQLSMALLINT  i = 1;

	while (true)
	{
		ret = SQLGetDiagRecW(handleType, handle, i, sqlState, &nativeError, messageText, sizeof(messageText) / sizeof(SQLWCHAR), &textLength);
		if (ret == SQL_NO_DATA)
			break;
		wcout.imbue(locale("kor"));
		wcout << L"[ODBC ERROR] SQLSTATE:" << sqlState << L"NativeError: %d" << nativeError << L"Message: %s" << messageText << endl;
		i++;
	}
}

bool DBConnection::Connect(SQLHENV henv, const WCHAR* connectionString)
{
	if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection) != SQL_SUCCESS)
		return false;

	WCHAR stringBuffer[MAX_PATH] = { 0 };
	::wcscpy_s(stringBuffer, connectionString);

	WCHAR resultString[MAX_PATH] = { 0 };
	SQLSMALLINT resultStringLen = 0;

	SQLRETURN ret = ::SQLDriverConnectW(
		_connection,
		NULL,
		reinterpret_cast<SQLWCHAR*>(stringBuffer),
		_countof(stringBuffer),
		OUT reinterpret_cast<SQLWCHAR*>(resultString),
		_countof(resultString),
		OUT &resultStringLen,
		SQL_DRIVER_NOPROMPT
	);

	if (ret == SQL_ERROR) // SQL_ERROR == -1
	{
		PrintOdbcError(SQL_HANDLE_DBC, _connection); // ← 에러 메시지 출력!
		return false;
	}

	if (::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement) != SQL_SUCCESS)
		return false;

	return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
		
}

void DBConnection::Clear()
{
	if (_connection != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
		_connection = SQL_NULL_HANDLE;
	}
	if (_statement != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _connection);
		_connection = SQL_NULL_HANDLE;
	}
}

bool DBConnection::Execute(const WCHAR* query, bool hasOutputParams)
{
	SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTS);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	{
		// OUTPUT 파라미터가 있는 경우에만 SQLMoreResults 호출
		if (hasOutputParams)
		{
			::SQLMoreResults(_statement);
		}
		
		return true;
	}

	HandleError(ret);
	return false;
}

bool DBConnection::Fetch()
{
	SQLRETURN ret = ::SQLFetch(_statement);

	switch (ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA:
		return false;
	case SQL_ERROR:
		HandleError(ret);
		return false;
	default:
		return true;

	}
}

int32 DBConnection::GetRowCount()
{
	SQLLEN count = 0;
	SQLRETURN ret = ::SQLRowCount(_statement, OUT &count);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return static_cast<int32>(count);

	return -1;
}

void DBConnection::Unbind()
{
	::SQLFreeStmt(_statement, SQL_UNBIND);
	::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
	::SQLFreeStmt(_statement, SQL_CLOSE);
}

bool DBConnection::BindParam(int32 paramIndex, bool* value, SQLLEN* index, SQLSMALLINT direction)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(bool), value, index, direction);
}

bool DBConnection::BindParam(int32 paramIndex, float* value, SQLLEN* index, SQLSMALLINT direction)
{
	return BindParam(paramIndex, SQL_C_FLOAT, SQL_REAL, 0, value, index, direction);
}

bool DBConnection::BindParam(int32 paramIndex, double* value, SQLLEN* index, SQLSMALLINT direction)
{
	return BindParam(paramIndex, SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index, direction);
}

bool DBConnection::BindParam(int32 paramIndex, int8* value, SQLLEN* index, SQLSMALLINT direction)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(int8), value, index, direction);
}

bool DBConnection::BindParam(int32 paramIndex, int16* value, SQLLEN* index, SQLSMALLINT direction)
{
	return BindParam(paramIndex, SQL_C_SHORT, SQL_SMALLINT, size32(int16), value, index, direction);
}

bool DBConnection::BindParam(int32 paramIndex, int32* value, SQLLEN* index, SQLSMALLINT direction)
{
	return BindParam(paramIndex, SQL_C_LONG, SQL_INTEGER, size32(int32), value, index, direction);
}

bool DBConnection::BindParam(int32 paramIndex, int64* value, SQLLEN* index, SQLSMALLINT direction)
{
	return BindParam(paramIndex, SQL_C_SBIGINT, SQL_BIGINT, size32(int64), value, index, direction);
}

bool DBConnection::BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index, SQLSMALLINT direction)
{
	return BindParam(paramIndex, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index, direction);
}

bool DBConnection::BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index, SQLSMALLINT direction)
{
	SQLULEN size = static_cast<SQLULEN>((::wcslen(str) + 1) * 2); // null문자  + wchar당 2니까 x2
	*index = SQL_NTSL;

	if (size > WVARCHAR_MAX) // 4000
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)str, index, direction);
	else
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)str, index, direction);
}

// 바이너리를 저장할 때 사용  
bool DBConnection::BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index, SQLSMALLINT direction)
{
	if (bin == nullptr)
	{
		*index = SQL_NULL_DATA;
		size = 1;
	}
	else
		*index = size;

	if (size > BINARY_MAX)
		return BindParam(paramIndex, SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*)bin, index, direction);
	else
		return BindParam(paramIndex, SQL_C_BINARY, SQL_BINARY, size, (BYTE*)bin, index, direction);
}

bool DBConnection::BindCol(int32 columnIndex, bool* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TINYINT, size32(bool), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, float* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_FLOAT, size32(float), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, double* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_DOUBLE, size32(double), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int8* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TINYINT, size32(int8), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int16* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_SHORT, size32(int16), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int32* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_LONG, size32(int32), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, int64* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_SBIGINT, size32(int64), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_C_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindCol(int32 columnIndex, WCHAR* str, int32 size, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_BINARY, size, str, index);
}

bool DBConnection::BindCol(int32 columnIndex, BYTE* bin, int32 size, SQLLEN* index)
{
	return BindCol(columnIndex, SQL_BINARY, size, bin, index);
}

bool DBConnection::BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLLEN len,
                             SQLPOINTER ptr, SQLLEN* index, SQLSMALLINT direction)
{
	// OUTPUT 파라미터의 경우 특별한 처리
	if (direction == SQL_PARAM_OUTPUT)
	{
		*index = 0; // OUTPUT 파라미터는 초기값을 0으로 설정
	}
	     
	SQLRETURN ret = ::SQLBindParameter(_statement, paramIndex, direction, cType, sqlType, len, 0, ptr, 0, index);
	
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		HandleError(ret);
		return false;
	}
	return true;
}

bool DBConnection::BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindCol(_statement, columnIndex, cType, value, len, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

void DBConnection::HandleError(SQLRETURN ret)
{
	if (ret == SQL_SUCCESS)
		return;

	SQLSMALLINT index = 1;
	SQLWCHAR sqlState[MAX_PATH] = { 0 };
	SQLINTEGER nativeErr = 0;
	SQLWCHAR errMsg[MAX_PATH] = {0};
	SQLSMALLINT msgLen = 0;
	SQLRETURN errorRet = 0;

	while (true)
	{
		::SQLGetDiagRecW(
			SQL_HANDLE_STMT,
			_statement,
			index,
			sqlState,
			OUT &nativeErr,
			errMsg,
			_countof(errMsg),
			OUT &msgLen
		);

		if (errorRet == SQL_NO_DATA)
			break;

		if (errorRet != SQL_SUCCESS && errorRet != SQL_SUCCESS_WITH_INFO)
			break;

		// TODO : Log
		wcout.imbue(locale("kor"));
		wcout << errMsg << endl;

		index++;
	}
}
