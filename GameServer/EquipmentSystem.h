#pragma once
#include "EquipmentCore.h"

class EquipmentSystem
{
public:
	EquipmentSystem();
	~EquipmentSystem() = default;

	// 장비 시스템 초기화
	void Initialize();
	void Clear();

	// 장비 장착/교체/제거
	void EquipItem(Protocol::EEquipmentSlotType slotType, int equipmentInstanceId);
	void UnequipItem(Protocol::EEquipmentSlotType slotType);

	// 장비 슬롯 관련
	bool IsSlotEmpty(Protocol::EEquipmentSlotType slotType) const;


private:
	Array<EquipmentSlot, EQUIPMENT_TOTAL_SLOTS> _slots;
};

