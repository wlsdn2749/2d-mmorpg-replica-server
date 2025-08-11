#pragma once
#include <future>
#include "DBDisPatcher.h"
#include "GenProcedures.h"

struct AccountRepository
{
	static void UpsertAccount_DB(DBConnection& conn, int32 userId)
	{
		SP::AccountsUpsert sp(conn);
		sp.ParamIn_UserId(userId);
		sp.Execute();
	}
	
	static std::future<void> UpsertAccountAsync(int32 userId)
	{
		return DbDispatcher::EnqueueRet([userId](DBConnection& c) {
			UpsertAccount_DB(c, userId);
		});
	}
};
