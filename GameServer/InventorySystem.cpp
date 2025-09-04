#include "pch.h"
#include "InventorySystem.h"
#include "ItemManager.h"
#include <algorithm>
#include <iostream>

InventorySystem::InventorySystem()
{
    Initialize();
}

void InventorySystem::Initialize()
{
    // 모든 슬롯을 빈 상태로 초기화
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
    {
        _slots[i].slotIndex = i;
        _slots[i].itemId = 0;
        _slots[i].count = 0;
        _slots[i].isQuickSlot = (i >= INVENTORY_NORMAL_SLOTS); // 30~39번 슬롯은 퀵슬롯
    }
}

void InventorySystem::Clear()
{
    for (auto& slot : _slots)
    {
        slot.Clear();
    }
    Initialize();
}

EAddItemResult InventorySystem::AddItem(int itemId, int count)
{
    if (itemId <= 0 || count <= 0)
        return EAddItemResult::InvalidItem;

    // ItemManager에서 아이템 데이터 확인
    const ItemData* itemData = ItemManager::Instance().GetItemData(itemId);
    if (!itemData) 
        return EAddItemResult::InvalidItem;

    // 기존 슬롯에 스택 가능한지 확인
    EAddItemResult stackResult = TryAddToExistingSlot(itemId, count);
    if (stackResult == EAddItemResult::Success)
        return EAddItemResult::Success;

    // 새로운 슬롯에 추가
    return TryAddToNewSlot(itemId, count);
}

ERemoveItemResult InventorySystem::RemoveItem(int slotIndex, int count)
{
    if (!IsValidSlotIndex(slotIndex))
        return ERemoveItemResult::InvalidSlot;

    ItemSlot& slot = _slots[slotIndex];
    if (slot.IsEmpty())
        return ERemoveItemResult::ItemNotFound;

    if (slot.count < count)
        return ERemoveItemResult::InsufficientQuantity;

    slot.count -= count;
    if (slot.count <= 0)
    {
        slot.Clear();
        slot.slotIndex = slotIndex;
        slot.isQuickSlot = (slotIndex >= INVENTORY_NORMAL_SLOTS);
    }

    return ERemoveItemResult::Success;
}

ERemoveItemResult InventorySystem::RemoveItemById(int itemId, int count)
{
    if (itemId <= 0 || count <= 0)
        return ERemoveItemResult::ItemNotFound;

    int remainingToRemove = count;

    // 모든 슬롯을 검색해서 해당 아이템을 찾아 제거
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS && remainingToRemove > 0; ++i)
    {
        ItemSlot& slot = _slots[i];
        if (slot.itemId == itemId && !slot.IsEmpty())
        {
            int removeFromThisSlot = std::min(remainingToRemove, slot.count);
            slot.count -= removeFromThisSlot;
            remainingToRemove -= removeFromThisSlot;

            if (slot.count <= 0)
            {
                slot.Clear();
                slot.slotIndex = i;
                slot.isQuickSlot = (i >= INVENTORY_NORMAL_SLOTS);
            }
        }
    }

    return (remainingToRemove == 0) ? ERemoveItemResult::Success : ERemoveItemResult::InsufficientQuantity;
}

EUseItemResult InventorySystem::UseItem(int slotIndex)
{
    if (!IsValidSlotIndex(slotIndex))
        return EUseItemResult::ItemNotFound;

    const ItemSlot& slot = _slots[slotIndex];
    if (slot.IsEmpty())
        return EUseItemResult::ItemNotFound;

    const ItemData* itemData = ItemManager::Instance().GetItemData(slot.itemId);
    if (!itemData || itemData->itemType != Protocol::EItemType::ITEM_TYPE_CONSUMABLE)
        return EUseItemResult::ItemNotUsable;

    // 아이템 효과 적용
    ApplyItemEffect(slot.itemId, 1);

    // 소비형 아이템은 사용 후 제거
    RemoveItem(slotIndex, 1);

    return EUseItemResult::Success;
}

const ItemSlot& InventorySystem::GetSlot(int slotIndex) const
{
    static ItemSlot emptySlot;
    if (!IsValidSlotIndex(slotIndex))
        return emptySlot;
    
    return _slots[slotIndex];
}

ItemSlot& InventorySystem::GetSlot(int slotIndex)
{
    static ItemSlot emptySlot;
    if (!IsValidSlotIndex(slotIndex))
        return emptySlot;
    
    return _slots[slotIndex];
}

bool InventorySystem::IsValidSlotIndex(int slotIndex) const
{
    return slotIndex >= 0 && slotIndex < INVENTORY_TOTAL_SLOTS;
}

bool InventorySystem::IsSlotEmpty(int slotIndex) const
{
    if (!IsValidSlotIndex(slotIndex))
        return true;
    
    return _slots[slotIndex].IsEmpty();
}

bool InventorySystem::SetQuickSlot(int slotIndex, bool isQuickSlot)
{
    if (!IsValidSlotIndex(slotIndex))
        return false;

    // 퀵슬롯 영역(30-39)만 퀵슬롯으로 설정 가능
    if (isQuickSlot && slotIndex < INVENTORY_NORMAL_SLOTS)
        return false;

    _slots[slotIndex].isQuickSlot = isQuickSlot;
    return true;
}

std::vector<int> InventorySystem::GetQuickSlotIndices() const
{
    std::vector<int> quickSlots;
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
    {
        if (_slots[i].isQuickSlot)
            quickSlots.push_back(i);
    }
    return quickSlots;
}

int InventorySystem::FindItemSlot(int itemId) const
{
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
    {
        if (_slots[i].itemId == itemId && !_slots[i].IsEmpty())
            return i;
    }
    return -1;
}

int InventorySystem::FindEmptySlot() const
{
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
    {
        if (_slots[i].IsEmpty())
            return i;
    }
    return -1;
}

std::vector<int> InventorySystem::GetItemSlots(int itemId) const
{
    std::vector<int> slots;
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
    {
        if (_slots[i].itemId == itemId && !_slots[i].IsEmpty())
            slots.push_back(i);
    }
    return slots;
}

int InventorySystem::GetUsedSlots() const
{
    int used = 0;
    for (const auto& slot : _slots)
    {
        if (!slot.IsEmpty())
            used++;
    }
    return used;
}

int InventorySystem::GetAvailableSlots() const
{
    return INVENTORY_TOTAL_SLOTS - GetUsedSlots();
}

bool InventorySystem::IsFull() const
{
    return GetAvailableSlots() == 0;
}

std::vector<Protocol::InventorySlotInfo> InventorySystem::ToProtocolSlots() const
{
    std::vector<Protocol::InventorySlotInfo> slots;
    for (const auto& slot : _slots)
    {
        if (!slot.IsEmpty())
        {
            slots.push_back(slot.ToProtocolSlotInfo());
        }
    }
    return slots;
}

void InventorySystem::FromInventorySlots(const std::vector<ItemSlot>& slots)
{
    Clear();
    
    for (const auto& slot : slots)
    {
        if (IsValidSlotIndex(slot.slotIndex) && !slot.IsEmpty())
        {
            _slots[slot.slotIndex] = slot;
        }
    }
}

void InventorySystem::PrintInventory() const
{
    std::cout << "=== Inventory Status ===" << std::endl;
    std::cout << "Used Slots: " << GetUsedSlots() << "/" << INVENTORY_TOTAL_SLOTS << std::endl;
    
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
    {
        const auto& slot = _slots[i];
        if (!slot.IsEmpty())
        {
            std::cout << "Slot[" << i << "]: ItemID=" << slot.itemId 
                     << ", Count=" << slot.count 
                     << ", Quick=" << (slot.isQuickSlot ? "Yes" : "No") << std::endl;
        }
    }
}

// Private helper functions
EAddItemResult InventorySystem::TryAddToExistingSlot(int itemId, int count)
{
    // ItemManager에서 스택 가능 여부 확인
    const ItemData* itemData = ItemManager::Instance().GetItemData(itemId);
    if (!itemData || !itemData->isStackable) 
        return EAddItemResult::InvalidItem;
    
    const int maxStack = itemData->maxStack;
    
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
    {
        ItemSlot& slot = _slots[i];
        if (slot.itemId == itemId && !slot.IsEmpty())
        {
            int availableSpace = maxStack - slot.count;
            if (availableSpace > 0)
            {
                int addAmount = std::min(count, availableSpace);
                slot.count += addAmount;
                count -= addAmount;
                
                if (count <= 0)
                    return EAddItemResult::Success;
            }
        }
    }
    
    return (count > 0) ? EAddItemResult::InventoryFull : EAddItemResult::Success;
}

EAddItemResult InventorySystem::TryAddToNewSlot(int itemId, int count)
{
    int emptySlotIndex = FindEmptySlot();
    if (emptySlotIndex == -1)
        return EAddItemResult::InventoryFull;

    ItemSlot& slot = _slots[emptySlotIndex];
    slot.itemId = itemId;
    slot.count = count;
    slot.slotIndex = emptySlotIndex;
    slot.isQuickSlot = (emptySlotIndex >= INVENTORY_NORMAL_SLOTS);

    return EAddItemResult::Success;
}

bool InventorySystem::CanStackItem(int itemId, int slotIndex, int additionalCount) const
{
    if (!IsValidSlotIndex(slotIndex))
        return false;

    const ItemSlot& slot = _slots[slotIndex];
    if (slot.itemId != itemId || slot.IsEmpty())
        return false;

    int maxStack = GetMaxStackSize(itemId);
    return (slot.count + additionalCount) <= maxStack;
}

int InventorySystem::GetMaxStackSize(int itemId) const
{
    const ItemData* itemData = ItemManager::Instance().GetItemData(itemId);
    return itemData ? itemData->maxStack : 1;
}

void InventorySystem::ApplyItemEffect(int itemId, int count)
{
    // TODO: 아이템 효과 시스템 구현
    // 포션 사용 시 HP 회복 등의 효과를 여기서 처리
    std::cout << "Applied effect for item " << itemId << " (count: " << count << ")" << std::endl;
}