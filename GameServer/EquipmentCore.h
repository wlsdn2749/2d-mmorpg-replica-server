#pragma once
#include "TypeCore.h"
#include "Protocol.pb.h"
#include <chrono>

using EquipmentSlotType	= Protocol::EEquipmentSlotType;

constexpr int EQUIPMENT_TOTAL_SLOTS = 4; // 0~3

// 장비 데이터 Sheet
struct EquipmentData
{
	int					itemId {};
	EquipmentSlotType	slotType {};
	int					minLevel {};
	int					atk {};
	int					def {};
	int					maxHp {};

	EquipmentData() = default;
	EquipmentData(int itemId, EquipmentSlotType slotType, int minLevel, int atk, int def, int maxHp)
		: itemId(itemId), slotType(slotType), minLevel(minLevel), atk(atk), def(def), maxHp(maxHp)
	{}
};

// 장비 메타 데이터 - DB
struct EquipmentInstance
{
	int			equipmentInstanceId {};
	int			itemId {};
	int			enhancementLevel {};
	std::chrono::system_clock::time_point	acquiredAt {};

	EquipmentInstance() = default;
	EquipmentInstance(int equipmentInstanceId, int itemId, int enhancementLevel, std::chrono::system_clock::time_point acquiredAt)
		: equipmentInstanceId(equipmentInstanceId), itemId(itemId),
		  enhancementLevel(enhancementLevel), acquiredAt(acquiredAt)
	{}
};

struct EquipmentSlot
{
	EquipmentSlotType	slotType {};
	int					equipmentInstanceId {}; // 0이면 비어있음
	int					itemId {}; // 장착된 아이템 ID (빠른 접근용)
	int					enhancementLevel {}; // 강화 레벨 (캐시)

	EquipmentSlot() = default;
	EquipmentSlot(EquipmentSlotType type, int instanceId = 0, int itemId = 0, int enhancementLevel = 0)
		: slotType(type), equipmentInstanceId(instanceId), itemId(itemId), enhancementLevel(enhancementLevel)
	{
	}

	// 비어있는 슬롯인지 확인
	bool IsEmpty() const { return itemId == 0; }

	// 슬롯 초기화
	void Clear()
	{
		equipmentInstanceId = 0;
		itemId = 0;
		enhancementLevel = 0;
	}
};


enum class EEquipItemResult {
	Success,
	AlreadyEquipped,
};