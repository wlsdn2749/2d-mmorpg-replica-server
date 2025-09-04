#include "pch.h"
#include "ItemManager.h"
#include "Player.h"
#include "GenProcedures.h"
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
    
    // 기본 아이템 데이터 추가 (DB 로드 실패 시 폴백용)
    AddItemData(ItemData{1, "Health Potion", "Restores 50 HP", true, 99, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    AddItemData(ItemData{2, "Mana Potion", "Restores 30 MP", true, 99, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    AddItemData(ItemData{3, "Iron Sword", "A basic iron sword", false, 1, Protocol::EItemType::ITEM_TYPE_EQUIPMENT});
    AddItemData(ItemData{4, "Quest Item", "Important quest item", false, 1, Protocol::EItemType::ITEM_TYPE_QUEST});
    
    // DB에서 아이템 데이터 로드 (동기 실행)
    try {
        auto future = LoadAllItemDataAsync();
        bool loadSuccess = future.get(); // 결과 대기
        
        if (loadSuccess) {
            GConsoleLogger->WriteStdOut(Color::GREEN, L"ItemManager: DB에서 아이템 데이터 로딩 성공\n");
        } else {
            GConsoleLogger->WriteStdOut(Color::YELLOW, L"ItemManager: DB 로딩 실패, 기본 데이터 사용\n");
        }
    } catch (const std::exception& e) {
        GConsoleLogger->WriteStdOut(Color::RED, L"ItemManager: DB 로딩 중 예외 발생, 기본 데이터 사용\n");
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

std::future<bool> ItemManager::LoadAllItemDataAsync()
{
    return DbDispatcher::EnqueueRet([this](DBConnection& c) {
        LoadAllItemData_DB(c);
        return true;
    });
}

void ItemManager::LoadAllItemData_DB(DBConnection& conn)
{
    GConsoleLogger->WriteStdOut(Color::YELLOW, L"ItemManager: Loading items from database...\n");
    
    // inventory-test-data.sql 파일의 데이터와 동일하게 로드
    _itemDataMap.clear();
    
    // TODO: Encoding Error? 
    // 소비형 아이템 (포션류) - itemType 1 = ITEM_TYPE_CONSUMABLE
    //AddItemData(ItemData{1, "체력 회복 포션", "HP를 50 회복시킵니다.", true, 99, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    //AddItemData(ItemData{2, "마나 회복 포션", "MP를 30 회복시킵니다.", true, 99, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    //AddItemData(ItemData{3, "고급 체력 포션", "HP를 100 회복시킵니다.", true, 50, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    //AddItemData(ItemData{4, "전투 자극제", "공격력을 일시적으로 증가시킵니다.", true, 20, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    //AddItemData(ItemData{5, "방어 물약", "방어력을 일시적으로 증가시킵니다.", true, 20, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    //
    //// 장비 아이템 - itemType 2 = ITEM_TYPE_EQUIPMENT
    //AddItemData(ItemData{10, "초보자 검", "새로운 모험가를 위한 기본 검입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_EQUIPMENT});
    //AddItemData(ItemData{11, "철검", "튼튼한 철로 제작된 검입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_EQUIPMENT});
    //AddItemData(ItemData{12, "은검", "아름다운 은으로 제작된 검입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_EQUIPMENT});
    //AddItemData(ItemData{13, "가죽 갑옷", "기본적인 방어력을 제공하는 가죽 갑옷입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_EQUIPMENT});
    //AddItemData(ItemData{14, "철갑옷", "높은 방어력을 자랑하는 철갑옷입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_EQUIPMENT});
    //AddItemData(ItemData{15, "마법 방패", "마법 공격을 막아주는 신비한 방패입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_EQUIPMENT});
    //
    //// 퀘스트 아이템 - itemType 3 = ITEM_TYPE_QUEST  
    //AddItemData(ItemData{20, "잃어버린 편지", "중요한 내용이 담긴 편지입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_QUEST});
    //AddItemData(ItemData{21, "고대 유물 조각", "고대 문명의 흔적이 담긴 신비한 조각입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_QUEST});
    //AddItemData(ItemData{22, "수상한 열쇠", "어떤 문을 열 수 있을지 모르는 열쇠입니다.", false, 1, Protocol::EItemType::ITEM_TYPE_QUEST});
    //
    //// 기타 아이템 - itemType 4로 가정 (ITEM_TYPE_MISC 없으면 ITEM_TYPE_CONSUMABLE 사용)
    //AddItemData(ItemData{30, "마을 귀환 주문서", "마을로 순간이동할 수 있는 주문서입니다.", true, 10, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    //AddItemData(ItemData{31, "던전 입장권", "특별한 던전에 입장할 수 있는 티켓입니다.", true, 5, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    //AddItemData(ItemData{32, "경험치 북", "사용하면 경험치를 획득할 수 있습니다.", true, 20, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    //AddItemData(ItemData{33, "금화 주머니", "소량의 금화가 들어있는 주머니입니다.", true, 99, Protocol::EItemType::ITEM_TYPE_CONSUMABLE});
    
    GConsoleLogger->WriteStdOut(Color::GREEN, L"ItemManager: Loaded %d items from database.\n", 
                               static_cast<int>(_itemDataMap.size()));
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

void ItemManager::ApplyItemEffect(int itemId, int count, Player* player)
{
    if (!player)
        return;
        
    const ItemData* data = GetItemData(itemId);
    if (!data || !CanUseItem(itemId))
        return;
    
    // 아이템 종류별 효과 적용
    switch (itemId)
    {
        case 1: // Health Potion
        case 5: // Super Health Potion  
            ApplyHealthPotionEffect(itemId, count, player);
            break;
            
        case 2: // Mana Potion
            ApplyManaPotionEffect(itemId, count, player);
            break;
            
        default:
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
void ItemManager::ApplyHealthPotionEffect(int itemId, int count, Player* player)
{
    int healAmount = 0;
    
    switch (itemId)
    {
        case 1: // Health Potion
            healAmount = 50 * count;
            break;
        case 5: // Super Health Potion
            healAmount = 100 * count;
            break;
        default:
            return;
    }
    
    // 현재 HP에 회복량 추가 (최대 HP를 넘지 않도록)
    int currentHp = player->Hp();
    int maxHp = player->_maxHp; // TODO: 나중에 getter로 변경
    int newHp = std::min(currentHp + healAmount, maxHp);
    
    player->_hp = newHp; // TODO: 나중에 setter로 변경
    
    GConsoleLogger->WriteStdOut(Color::GREEN, L"Player healed for %d HP (from %d to %d)\n", 
                               healAmount, currentHp, newHp);
}

void ItemManager::ApplyManaPotionEffect(int itemId, int count, Player* player)
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