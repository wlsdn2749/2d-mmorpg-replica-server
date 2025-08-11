#pragma once

class DBConnGuard : Uncopyable
{
public:
	explicit DBConnGuard(DBConnectionPool& pool = *GDBConnectionPool) 
		: _pool(&pool)
		, _conn(nullptr)
	{
		ASSERT_CRASH(_conn = _pool->Pop());
	}
	~DBConnGuard() 
	{
		release();
	}

	void release()
	{
		if (_pool && _conn)
		{
			_pool->Push(_conn);
			_conn = nullptr;
		}
	}

public:
	DBConnection* get() const { return _conn; }
	DBConnection& ref() const { return *_conn; }

private:
	DBConnectionPool*	_pool { nullptr };
	DBConnection*		_conn { nullptr };
};
