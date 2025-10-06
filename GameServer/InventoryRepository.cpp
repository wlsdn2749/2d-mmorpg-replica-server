#include "pch.h"
#include "InventoryRepository.h"

std::vector<ItemSlot> InventoryRepository::GetCharacterInventory_DB(DBConnection& conn, int characterId)
{
    std::vector<ItemSlot> slots;

    int slotIndex;
    int itemId;
    int count;
    int isQuickslot;
    int equipmentInstanceId;

    SP::GetCharacterInventory sp(conn);
    sp.ParamIn_CharacterId(characterId);
    sp.ColumnOut_SlotIndex(OUT slotIndex);
    sp.ColumnOut_ItemId(OUT itemId);
    sp.ColumnOut_Count(OUT count);
    sp.ColumnOut_IsQuickslot(OUT isQuickslot);
    sp.ColumnOut_EquipmentInstanceId(OUT equipmentInstanceId);

    sp.Execute();

    while (sp.Fetch())
    {
        ItemSlot slot;
        slot.slotIndex = slotIndex;
        slot.itemId = itemId;
        slot.count = count;
        slot.isQuickSlot = (isQuickslot != 0);
        slot.equipmentInstanceId = equipmentInstanceId;

        slots.push_back(slot);

        GConsoleLogger->WriteStdOut(Color::GREEN,
            L"Loaded inventory slot - CharID[%d] Slot[%d] ItemID[%d] Count[%d] QuickSlot[%s] EquipInstID[%d]\n",
            characterId, slotIndex, itemId, count, slot.isQuickSlot ? L"Yes" : L"No", equipmentInstanceId);
    }

    return slots;
}

std::future<std::vector<ItemSlot>> InventoryRepository::GetCharacterInventoryAsync(int characterId)
{
    return DbDispatcher::EnqueueRet([characterId](DBConnection& c) {
        return GetCharacterInventory_DB(c, characterId);
    });
}

void InventoryRepository::SaveInventorySlot_DB(DBConnection& conn, int characterId, const ItemSlot& slot)
{
    SP::SaveInventorySlot sp(conn);

    // const_cast 사용: BindParam은 읽기 전용이므로 안전
    sp.ParamIn_CharacterId(const_cast<int32&>(characterId));
    sp.ParamIn_SlotIndex(const_cast<int32&>(slot.slotIndex));
    sp.ParamIn_ItemId(const_cast<int32&>(slot.itemId));
    sp.ParamIn_Count(const_cast<int32&>(slot.count));

    // bool을 int로 변환
    int isQuickslot = slot.isQuickSlot ? 1 : 0;
    sp.ParamIn_IsQuickslot(isQuickslot);
    sp.ParamIn_EquipmentInstanceId(const_cast<int32&>(slot.equipmentInstanceId));

    sp.Execute();

    GConsoleLogger->WriteStdOut(Color::WHITE,
        L"Saved inventory slot - CharID[%d] Slot[%d] ItemID[%d] Count[%d] QuickSlot[%s] EquipInstID[%d]\n",
        characterId, slot.slotIndex, slot.itemId, slot.count, slot.isQuickSlot ? L"Yes" : L"No", slot.equipmentInstanceId);
}

std::future<void> InventoryRepository::SaveInventorySlotAsync(int characterId, const ItemSlot& slot)
{
    return DbDispatcher::Enqueue([characterId, slotCopy = slot](DBConnection& c) {
        SaveInventorySlot_DB(c, characterId, slotCopy);
    });
}

void InventoryRepository::DeleteInventorySlot_DB(DBConnection& conn, int characterId, int slotIndex)
{
    SP::DeleteInventorySlot sp(conn);
    
    sp.ParamIn_CharacterId(characterId);
    sp.ParamIn_SlotIndex(slotIndex);
    
    sp.Execute();
    
    GConsoleLogger->WriteStdOut(Color::YELLOW,
        L"Deleted inventory slot - CharID[%d] Slot[%d]\n", 
        characterId, slotIndex);
}

std::future<void> InventoryRepository::DeleteInventorySlotAsync(int characterId, int slotIndex)
{
    return DbDispatcher::Enqueue([characterId, slotIndex](DBConnection& c) {
        DeleteInventorySlot_DB(c, characterId, slotIndex);
    });
}

void InventoryRepository::SaveFullInventory_DB(DBConnection& conn, int characterId, const std::vector<ItemSlot>& slots)
{
    // 트랜잭션 시작
    // TODO: 트랜잭션 관리 코드 추가 필요
    
    GConsoleLogger->WriteStdOut(Color::GREEN,
        L"Saving full inventory - CharID[%d] SlotCount[%d]\n", 
        characterId, static_cast<int>(slots.size()));
    
    // 기존 인벤토리 데이터를 모두 삭제 (선택적)
    // ClearCharacterInventory_DB(conn, characterId);
    
    // 모든 슬롯을 하나씩 저장
    for (const auto& slot : slots)
    {
        if (!slot.IsEmpty())
        {
            SaveInventorySlot_DB(conn, characterId, slot);
        }
    }
    
    GConsoleLogger->WriteStdOut(Color::GREEN,
        L"Full inventory save complete - CharID[%d]\n", characterId);
}

std::future<void> InventoryRepository::SaveFullInventoryAsync(int characterId, const std::vector<ItemSlot>& slots)
{
    return DbDispatcher::Enqueue([characterId, slotsCopy = slots](DBConnection& c) {
        SaveFullInventory_DB(c, characterId, slotsCopy);
    });
}

void InventoryRepository::ClearCharacterInventory_DB(DBConnection& conn, int characterId)
{
    // 해당 캐릭터의 모든 인벤토리 슬롯 삭제
    for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
    {
        DeleteInventorySlot_DB(conn, characterId, i);
    }
    
    GConsoleLogger->WriteStdOut(Color::RED,
        L"Cleared all inventory slots - CharID[%d]\n", characterId);
}

std::future<void> InventoryRepository::ClearCharacterInventoryAsync(int characterId)
{
    return DbDispatcher::Enqueue([characterId](DBConnection& c) {
        ClearCharacterInventory_DB(c, characterId);
    });
}