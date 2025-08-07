#pragma once
#include <sql.h>
#include <sqlext.h>

/*--------------------
	DBConnection
---------------------*/
enum
{
	WVARCHAR_MAX = 4000,
	BINARY_MAX = 8000,
};
class DBConnection
{
public:
	bool Connect(SQLHENV henv, const WCHAR* connectionString);
	void Clear();

	bool Execute(const WCHAR* query, bool hasOutputParams = false);
	bool Fetch();

	int32 GetRowCount();
	void Unbind();

public:
	bool	BindParam(int32 paramIndex, bool* value, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT); // bool도 지원 X
	bool	BindParam(int32 paramIndex, float* value, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT);
	bool	BindParam(int32 paramIndex, double* value, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT);
	bool	BindParam(int32 paramIndex, int8* value, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT); // Unsigned int DB에서 지원 X
	bool	BindParam(int32 paramIndex, int16* value, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT);
	bool	BindParam(int32 paramIndex, int32* value, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT);
	bool	BindParam(int32 paramIndex, int64* value, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT);
	bool	BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT);
	bool	BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT);
	bool	BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index, SQLSMALLINT direction = SQL_PARAM_INPUT);

	bool	BindCol(int32 columnIndex, bool* value, SQLLEN* index); // bool도 지원 X
	bool	BindCol(int32 columnIndex, float* value, SQLLEN* index);
	bool	BindCol(int32 columnIndex, double* value, SQLLEN* index);
	bool	BindCol(int32 columnIndex, int8* value, SQLLEN* index); // Unsigned int DB에서 지원 X
	bool	BindCol(int32 columnIndex, int16* value, SQLLEN* index);
	bool	BindCol(int32 columnIndex, int32* value, SQLLEN* index);
	bool	BindCol(int32 columnIndex, int64* value, SQLLEN* index);
	bool	BindCol(int32 columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool	BindCol(int32 columnIndex, WCHAR* str, int32 size, SQLLEN* index);
	bool	BindCol(int32 columnIndex, BYTE* bin, int32 size, SQLLEN* index);

private:
	bool BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLLEN len, SQLPOINTER ptr, SQLLEN* index, SQLSMALLINT direction);
	bool BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);
	void HandleError(SQLRETURN ret);
private:
	SQLHDBC		_connection = SQL_NULL_HANDLE;
	SQLHSTMT	_statement = SQL_NULL_HANDLE;
};

