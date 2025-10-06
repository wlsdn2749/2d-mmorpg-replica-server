#pragma once
#include "TypeCore.h"
#include "Protocol.pb.h"

// 인벤토리 관련 상수
constexpr int INVENTORY_TOTAL_SLOTS = 40;
constexpr int INVENTORY_NORMAL_SLOTS = 30;
constexpr int INVENTORY_QUICK_SLOTS = 10;

// 아이템 메타데이터 (Sheet Item 테이블에서 로드)
struct ItemData {
    int itemId = 0;
    std::string name;
    std::string description;
    bool isStackable = false;
    int maxStack = 1;
    Protocol::EItemType itemType = Protocol::EItemType::ITEM_TYPE_UNKNOWN;

    ItemData() = default;
    ItemData(int id, const std::string& n, const std::string& desc, 
             bool stackable, int maxStk, Protocol::EItemType type)
        : itemId(id), name(n), description(desc), 
          isStackable(stackable), maxStack(maxStk), itemType(type) {}
};

// 인벤토리 슬롯 정보
struct ItemSlot {
    int slotIndex = -1;
    int itemId = 0;
    int count = 0;
    bool isQuickSlot = false;
    int equipmentInstanceId = 0; // For equipment items only

    ItemSlot() = default;
    ItemSlot(int slot, int id, int cnt, bool quick = false, int equipInstId = 0)
        : slotIndex(slot), itemId(id), count(cnt), isQuickSlot(quick), equipmentInstanceId(equipInstId) {}

    // 비어있는 슬롯인지 확인
    bool IsEmpty() const { return itemId == 0 || count <= 0; }

    // 슬롯 초기화
    void Clear() {
        slotIndex = -1;
        itemId = 0;
        count = 0;
        isQuickSlot = false;
        equipmentInstanceId = 0;
    }

    // 프로토콜 메시지로 변환
    Protocol::InventorySlotInfo ToProtocolSlotInfo() const {
        Protocol::InventorySlotInfo slotInfo;
        slotInfo.set_slotindex(slotIndex);
        slotInfo.set_itemid(itemId);
        slotInfo.set_count(count);
        slotInfo.set_isquickslot(isQuickSlot);
        slotInfo.set_equipmentinstanceid(equipmentInstanceId);
        return slotInfo;
    }

    // 프로토콜 메시지에서 변환
    static ItemSlot FromProtocolSlotInfo(const Protocol::InventorySlotInfo& info) {
        return ItemSlot(info.slotindex(), info.itemid(), info.count(), info.isquickslot(), info.equipmentinstanceid());
    }
};

// 아이템 추가 결과
enum class EAddItemResult {
    Success,
    InventoryFull,
    InvalidItem,
    ExceedsMaxStack
};

const std::unordered_map<EAddItemResult, std::string> addItemResultToString = 
{
    {EAddItemResult::Success, "Success"},
    {EAddItemResult::InventoryFull, "InventoryFull"},
    {EAddItemResult::InvalidItem, "InvalidItem"},
    {EAddItemResult::ExceedsMaxStack, "ExceedsMaxStack"}
};

static inline std::string toString(EAddItemResult addItemResult)
{
    auto it = addItemResultToString.find(addItemResult);
    return (it != addItemResultToString.end()) ? it->second : "Unknown";
}

// 아이템 제거 결과  
enum class ERemoveItemResult {
    Success,
    ItemNotFound,
    InsufficientQuantity,
    InvalidSlot
};

// 아이템 사용 결과
enum class EUseItemResult {
    Success,
    ItemNotFound,
    ItemNotUsable,
    CooldownActive,
    InvalidCondition
};