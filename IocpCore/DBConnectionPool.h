#pragma once
#include "DBConnection.h"

/*--------------------
	DBConnectionPool
---------------------*/

class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	bool Connect(int32 connectionCount, const WCHAR* connectionString);
	void Clear();

	DBConnection*	Pop(); // Pop하고 사용한다음에
	void			Push(DBConnection* connection); // 여기서 반납 바로하므로, 굳이 smart_pointer 필요없음

private:
	USE_LOCK;
	SQLHENV 				_environment = SQL_NULL_HANDLE;
	Vector<DBConnection*>	_connections;
};

