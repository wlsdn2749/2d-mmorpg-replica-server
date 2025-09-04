#pragma once
#include "InventoryCore.h"
#include "DBDisPatcher.h"
#include "GenProcedures.h"
#include <future>
#include <vector>

struct InventoryRepository
{
    // 인벤토리 데이터 로드
    static std::vector<ItemSlot> GetCharacterInventory_DB(DBConnection& conn, int characterId);
    static std::future<std::vector<ItemSlot>> GetCharacterInventoryAsync(int characterId);
    
    // 인벤토리 슬롯 저장
    static void SaveInventorySlot_DB(DBConnection& conn, int characterId, const ItemSlot& slot);
    static std::future<void> SaveInventorySlotAsync(int characterId, const ItemSlot& slot);
    
    // 인벤토리 슬롯 삭제
    static void DeleteInventorySlot_DB(DBConnection& conn, int characterId, int slotIndex);
    static std::future<void> DeleteInventorySlotAsync(int characterId, int slotIndex);
    
    // 전체 인벤토리 저장 (배치 처리)
    static void SaveFullInventory_DB(DBConnection& conn, int characterId, const std::vector<ItemSlot>& slots);
    static std::future<void> SaveFullInventoryAsync(int characterId, const std::vector<ItemSlot>& slots);
    
    // 캐릭터의 모든 인벤토리 삭제 (캐릭터 삭제 시 사용)
    static void ClearCharacterInventory_DB(DBConnection& conn, int characterId);
    static std::future<void> ClearCharacterInventoryAsync(int characterId);
};