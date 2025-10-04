#pragma once
#include "TypeCore.h"
#include "Protocol.pb.h"
#include <chrono>

using time_point		= std::chrono::system_clock::time_point;
using EquipmentSlotType	= Protocol::EEquipmentSlotType;
// 장비 데이터 Sheet
struct EquipmentData
{
	int itemId {};
	EquipmentSlotType slotType {};
	int minLevel {};
	int atk {};
	int def {};
	int maxHp {};

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
	time_point	acquiredAt {};

	EquipmentInstance() = default;
	EquipmentInstance(int equipmentInstanceId, int itemId, int enhancementLevel, time_point acquiredAt)
		: equipmentInstanceId(equipmentInstanceId), itemId(itemId),
		  enhancementLevel(enhancementLevel), acquiredAt(acquiredAt)
	{}
};