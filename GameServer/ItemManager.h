#pragma once
#include "InventoryCore.h"
#include "DBDisPatcher.h"

#include "ItemDataParser.h"
#include <unordered_map>
#include <memory>
#include <future>

class ItemManager
{
#pragma region Meyers Singleton
public:
    static ItemManager& Instance();

    ItemManager(const ItemManager&) = delete;
    ItemManager& operator=(const ItemManager&) = delete;
private:
    ItemManager() = default;
    ~ItemManager() = default;

#pragma endregion

public:
    // 초기화 및 정리
    bool Initialize();
    void Shutdown();

    // 아이템 데이터 조회
    const ItemData* GetItemData(int itemId) const;
    bool IsValidItem(int itemId) const;
    bool IsStackableItem(int itemId) const;
    int GetMaxStackSize(int itemId) const;
    Protocol::EItemType GetItemType(int itemId) const;

    // 아이템 효과 처리
    bool CanUseItem(int itemId) const;
    void ApplyItemEffect(int itemId, int count, PlayerRef player);

    // 디버그 및 관리
    void AddItemData(const ItemData& itemData);
    void RemoveItemData(int itemId);
    void PrintAllItems() const;
    size_t GetItemCount() const;

private:
    // 아이템 효과 적용 헬퍼
    void ApplyHealthPotionEffect(int itemId, int count, PlayerRef player);
    void ApplyManaPotionEffect(int itemId, int count, PlayerRef player);

private:
    std::unordered_map<int, std::unique_ptr<ItemData>> _itemDataMap;
    bool _initialized = false;
};

// 편의를 위한 전역 접근 함수들
namespace ItemManagerGlobal
{
    const ItemData* GetItemData(int itemId);
    bool IsStackableItem(int itemId);
    int GetMaxStackSize(int itemId);
    Protocol::EItemType GetItemType(int itemId);
}