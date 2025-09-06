#include "pch.h"
#include "DropManager.h"

#include "DropTableRepository.h"
#include "ItemManager.h"

#include "RandomUtils.h"

void DropManager::LoadAllDropTableData()
{
	auto fut = DropTableRepository::GetMonsterItemsAsync();

	auto dropItemInfos = fut.get();

	for (auto& dropItemInfo : dropItemInfos)
	{
		auto result = AddDropItemData(std::move(dropItemInfo));
		if (result == false)
		{
			GConsoleLogger->WriteStdOut(Color::RED, L"AddDropItemData Failure - ID[%d]\n");
		}
	}

}

MonsterDropTable* DropManager::GetDropTable(int monsterId) const
{
	auto it = _dropDataMap.find(monsterId);
	return it != _dropDataMap.end() ? it->second.get() : nullptr;
}

Vector<DroppedItem> DropManager::ProcessAllDrops(int monsterId) const
{
	Vector<DroppedItem> droppedItems;

	const auto* dropTable = GetDropTable(monsterId);
	ASSERT_CRASH(dropTable != nullptr);
	
	for (const auto& dropItemInfo : dropTable->dropItemInfo)
	{
		if (RollDrop(dropItemInfo))
		{
			DroppedItem item {};
			item.count = GetRandomDropCount(dropItemInfo);
			item.itemId = dropItemInfo.itemId;

			droppedItems.push_back(std::move(item));
		}
	}
	return droppedItems;
}

bool DropManager::AddDropItemData(DropItemInfo&& dropItemInfo)
{
	const int id = dropItemInfo.id;
	const int monsterId = dropItemInfo.monsterId;
	const int itemId = dropItemInfo.itemId;
	const int dropRate = dropItemInfo.dropRate;
	const int minCount = dropItemInfo.minCount;
	const int maxCount = dropItemInfo.maxCount;
	const int isEnabled = dropItemInfo.isEnabled;

	// TODO 데이터 검증 및 예외 처리
	// TODO monsterId가 유효한가?
	
	if (ItemManager::Instance().IsValidItem(itemId) == false)
		return false;

	// dropRate가 0~100 범위 내에 있는지
	if (!(0 <= dropRate && dropRate <= 100))
		return false;

	// minCount가 0보다 크고, MaxCount보다 작은지
	if (!(0 < minCount && 0 < maxCount && minCount <= maxCount))
		return false;

	// isEnabled가 True인지
	if (!isEnabled)
		return false;

	_dropDataMap[monsterId]->monsterId = monsterId;
	_dropDataMap[monsterId]->dropItemInfo.push_back(std::move(dropItemInfo));
	
	GConsoleLogger->WriteStdOut(Color::YELLOW,
		L"AddDropItemData - ID[%d] MonsterId[%d] ItemId[%d] DropRate[%d] minCount[%d] maxCount[%d] isEnabled[%s]\n",
		id, monsterId, itemId, dropRate, minCount, maxCount, isEnabled ? L"Yes" : L"No");
	
	return true;
}

bool DropManager::RollDrop(const DropItemInfo& dropInfo) const
{
	return RandomUtils::GetRandomFloat100() < dropInfo.dropRate; 
}

int DropManager::GetRandomDropCount(const DropItemInfo& dropInfo) const
{
	const int minCount = dropInfo.minCount;
	const int maxCount = dropInfo.maxCount;
	return RandomUtils::GetRandom(minCount, maxCount);
}
