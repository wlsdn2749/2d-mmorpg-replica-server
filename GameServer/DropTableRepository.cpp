#include "pch.h"
#include "DropTableRepository.h"

Vector<DropItemInfo> DropTableRepository::GetMonsterItems_DB(DBConnection& conn)
{
	Vector<DropItemInfo> infos;

	int id;
	int monsterId;
	int itemId;
	int dropRate;
	int minCount;
	int maxCount;
	int isEnabled;

	SP::GetMonsterDropItems sp(conn);
	sp.ColumnOut_Id(OUT id);
	sp.ColumnOut_MonsterId(OUT monsterId);
	sp.ColumnOut_ItemId(OUT itemId);
	sp.ColumnOut_DropRate(OUT dropRate);
	sp.ColumnOut_MinCount(OUT minCount);
	sp.ColumnOut_MaxCount(OUT maxCount);
	sp.ColumnOut_IsEnabled(OUT isEnabled);

	sp.Execute(); 
	while (sp.Fetch())
	{
		DropItemInfo info;
		info.id = id;
		info.monsterId = monsterId;
		info.itemId = itemId;
		info.dropRate = dropRate;
		info.minCount = minCount;
		info.maxCount = maxCount;
		info.isEnabled = isEnabled;

		infos.push_back(info);

		GConsoleLogger->WriteStdOut(Color::GREEN,
			L"Loaded DropData - ID[%d] MonsterId[%d] ItemId[%d] DropRate[%d] minCount[%d] maxCount[%d] isEnabled[%s]\n",
			id,monsterId, itemId, dropRate, minCount, maxCount, isEnabled ? L"Yes" : L"No");
	}

	return infos;
}

std::future<Vector<DropItemInfo>> DropTableRepository::GetMonsterItemsAsync()
{

	return DbDispatcher::EnqueueRet([](DBConnection& c){
		return GetMonsterItems_DB(c);
	});
}
