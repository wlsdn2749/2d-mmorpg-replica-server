#pragma once
#include "DropManager.h"

struct DropTableRepository
{
	static Vector<DropItemInfo> GetMonsterItems_DB(DBConnection& conn);
	static std::future<Vector<DropItemInfo>> GetMonsterItemsAsync();
};

