#pragma once

struct DropItemInfo;

struct MonsterDropTable
{
	int monsterId;
	std::vector<DropItemInfo> dropItemInfo;
};

struct DropItemInfo
{
	int id {};
	int monsterId {};
	int itemId {};
	int dropRate {};
	int minCount {};
	int maxCount {};
	int isEnabled {};
};

// 아이템 View와 비슷
struct DroppedItem
{
	int itemId {};
	int count {};
};

// unordered_map<monsterId, vector<MonsterDropTable>>

class DropManager
{
#pragma region Meyers Singleton
public:
	static DropManager& Instance()
	{
		static DropManager dropManager;
		return dropManager;
	}

	DropManager(const DropManager&) = delete;
	DropManager& operator=(const DropManager&) = delete;
private:
	DropManager() = default;
	~DropManager() = default;

#pragma endregion

public:
	bool Initialize();

	// 드랍 테이블 로딩 함수
	void LoadAllDropTableData();

	// 드랍 아이템 리스트 조회 함수
	MonsterDropTable* GetDropTable(int monsterId) const;

	// monsterId의 1회성 드랍 체크
	Vector<DroppedItem> ProcessAllDrops(int monsterId) const;

	// 헬퍼
	bool AddDropItemData(DropItemInfo&& dropItemInfo);
	bool RollDrop(const DropItemInfo& dropInfo) const;
	int GetRandomDropCount(const DropItemInfo& dropInfo) const;

private:
	std::unordered_map<int, std::unique_ptr<MonsterDropTable>> _dropDataMap;
	bool _initialized = false;

};

