#include "pch.h"
#include "ItemManager.h"
#include "Player.h"
#include "GenProcedures.h"

#include "Room.h"

#include <iostream>
#include <algorithm>

ItemManager& ItemManager::Instance()
{
    static ItemManager instance;
    return instance;
}

bool ItemManager::Initialize()
{
    if (_initialized)
        return true;

    GConsoleLogger->WriteStdOut(Color::YELLOW, L"ItemManager: Initializing...\n");
    
    // 아이템 Load From Json
    auto itemDataMap = ItemDataParser::LoadItemData();

    for (const auto& [itemId, itemData] : itemDataMap)
    {
        AddItemData(itemData);
    }
    
    _initialized = true;
    
    GConsoleLogger->WriteStdOut(Color::GREEN, L"ItemManager: 초기화 완료. 총 %d개 아이템 로드됨.\n", 
                               static_cast<int>(_itemDataMap.size()));
    
    return true;
}

void ItemManager::Shutdown()
{
    if (!_initialized)
        return;

    GConsoleLogger->WriteStdOut(Color::YELLOW, L"ItemManager: Shutting down...\n");
    
    _itemDataMap.clear();
    _initialized = false;
    
    GConsoleLogger->WriteStdOut(Color::GREEN, L"ItemManager: Shutdown complete.\n");
}

const ItemData* ItemManager::GetItemData(int itemId) const
{
    auto it = _itemDataMap.find(itemId);
    return (it != _itemDataMap.end()) ? it->second.get() : nullptr;
}

bool ItemManager::IsValidItem(int itemId) const
{
    return _itemDataMap.find(itemId) != _itemDataMap.end();
}

bool ItemManager::IsStackableItem(int itemId) const
{
    const ItemData* data = GetItemData(itemId);
    return data ? data->isStackable : false;
}

int ItemManager::GetMaxStackSize(int itemId) const
{
    const ItemData* data = GetItemData(itemId);
    return data ? data->maxStack : 1;
}

Protocol::EItemType ItemManager::GetItemType(int itemId) const
{
    const ItemData* data = GetItemData(itemId);
    return data ? data->itemType : Protocol::EItemType::ITEM_TYPE_UNKNOWN;
}

bool ItemManager::CanUseItem(int itemId) const
{
    const ItemData* data = GetItemData(itemId);
    if (!data)
        return false;
    
    // 소비형 아이템만 사용 가능
    return data->itemType == Protocol::EItemType::ITEM_TYPE_CONSUMABLE;
}

void ItemManager::ApplyItemEffect(int itemId, int count, PlayerRef player)
{
    if (!player)
        return;
        
    const ItemData* data = GetItemData(itemId);
    if (!data || !CanUseItem(itemId))
        return;
    
    // 아이템 종류별 효과 적용
    switch (itemId)
    {
        case 10001: // Health Potion
        case 10002: // 고급 Health Potion  
            ApplyHealthPotionEffect(itemId, count, player);
            break;
            
        [[unlikely]] default:
            GConsoleLogger->WriteStdOut(Color::YELLOW, L"ItemManager: No effect defined for item %d\n", itemId);
            break;
    }
}

void ItemManager::AddItemData(const ItemData& itemData)
{
    _itemDataMap[itemData.itemId] = std::make_unique<ItemData>(itemData);
}

void ItemManager::RemoveItemData(int itemId)
{
    _itemDataMap.erase(itemId);
}

void ItemManager::PrintAllItems() const
{
    GConsoleLogger->WriteStdOut(Color::GREEN, L"=== ItemManager: All Items ===\n");
    
    for (const auto& pair : _itemDataMap)
    {
        const ItemData* data = pair.second.get();
        GConsoleLogger->WriteStdOut(Color::WHITE, 
            L"ID: %d, Name: %S, Stackable: %s, MaxStack: %d, Type: %d\n",
            data->itemId,
            data->name.c_str(),
            data->isStackable ? L"Yes" : L"No",
            data->maxStack,
            static_cast<int>(data->itemType));
    }
    
    GConsoleLogger->WriteStdOut(Color::GREEN, L"Total items: %d\n", static_cast<int>(_itemDataMap.size()));
}

size_t ItemManager::GetItemCount() const
{
    return _itemDataMap.size();
}

// Private helper functions
void ItemManager::ApplyHealthPotionEffect(int itemId, int count, PlayerRef player)
{
    int healAmount = 0;
    
    switch (itemId)
    {
        case 10001: // Health Potion
            healAmount = 30 * count;
            break;
        case 10002: // 고급 Health Potion
            healAmount = 50 * count;
            break;
        default:
            return;
    }
    
    // 현재 HP에 회복량 추가 (최대 HP를 넘지 않도록)
    int currentHp = player->Hp();
    int maxHp = player->MaxHp();
    int newHp = std::min(currentHp + healAmount, maxHp);
    
    player->SetHp(newHp);
    
    GConsoleLogger->WriteStdOut(Color::GREEN, L"Player healed for %d HP (from %d to %d)\n", 
                               healAmount, currentHp, newHp);

    auto room = player->GetRoom();
    room->DoAsync(&Room::OnPlayerHpChanged, player->playerId);
}

void ItemManager::ApplyManaPotionEffect(int itemId, int count, PlayerRef player)
{
    // TODO: MP 시스템이 구현되면 활성화
    int manaAmount = 30 * count;
    
    GConsoleLogger->WriteStdOut(Color::BLUE, L"Player recovered %d MP (MP system not implemented)\n", manaAmount);
}

// 편의를 위한 전역 접근 함수들
namespace ItemManagerGlobal
{
    const ItemData* GetItemData(int itemId)
    {
        return ItemManager::Instance().GetItemData(itemId);
    }
    
    bool IsStackableItem(int itemId)
    {
        return ItemManager::Instance().IsStackableItem(itemId);
    }
    
    int GetMaxStackSize(int itemId)
    {
        return ItemManager::Instance().GetMaxStackSize(itemId);
    }
    
    Protocol::EItemType GetItemType(int itemId)
    {
        return ItemManager::Instance().GetItemType(itemId);
    }
}