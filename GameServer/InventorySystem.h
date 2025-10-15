#pragma once
#include "InventoryCore.h"
#include <array>
#include <vector>

class ItemManager;

class InventorySystem
{
public:
    InventorySystem();
    ~InventorySystem() = default;

    // 인벤토리 초기화
    void Initialize();
    void Clear();

    // 아이템 추가/제거
    EAddItemResult AddItem(int itemId, int count, int equipmentInstanceId = 0);
    ERemoveItemResult RemoveItem(int slotIndex, int count);
    ERemoveItemResult RemoveItemById(int itemId, int count);

    // 아이템 사용
    EUseItemResult UseItem(int slotIndex);

    // 슬롯 관련
    const ItemSlot& GetSlot(int slotIndex) const;
    ItemSlot& GetSlot(int slotIndex);
    bool IsValidSlotIndex(int slotIndex) const;
    bool IsSlotEmpty(int slotIndex) const;
	Vector<ItemSlot*> GetSlotsById(int itemId);

    // 퀵슬롯 관리
    bool SetQuickSlot(int slotIndex, bool isQuickSlot);
    std::vector<int> GetQuickSlotIndices() const;

    // 검색
	int FindItemCount(int itemId) const;
    int FindItemSlot(int itemId) const;
    int FindEmptySlot() const;
    std::vector<int> GetItemSlots(int itemId) const;

    // 용량 관리
    int GetUsedSlots() const;
    int GetAvailableSlots() const;
    bool IsFull() const;

    // 데이터 변환
    std::vector<Protocol::InventorySlotInfo> ToProtocolSlots() const;
    void FromInventorySlots(const std::vector<ItemSlot>& slots);

    // 디버그
    void PrintInventory() const;

private:
    // 내부 헬퍼 함수들
    EAddItemResult TryAddToExistingSlot(int itemId, int count);
    EAddItemResult TryAddToNewSlot(int itemId, int count, int equipmentInstanceId);
    bool CanStackItem(int itemId, int slotIndex, int additionalCount) const;
    int GetMaxStackSize(int itemId) const;


private:
    std::array<ItemSlot, INVENTORY_TOTAL_SLOTS> _slots;
};