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
	EEquipItemResult EquipItem(Protocol::EEquipmentSlotType slotType, int itemId, int equipmentInstanceId = 0, int enhancementLevel = 0);
	void UnequipItem(Protocol::EEquipmentSlotType slotType);

	// 장비 슬롯 관련
	bool IsSlotEmpty(Protocol::EEquipmentSlotType slotType) const;

	// 장비 정보 확인
	const EquipmentSlot& GetEquipmentSlot(Protocol::EEquipmentSlotType slotType) const;
	EquipmentSlot GetEquipmentSlot(Protocol::EEquipmentSlotType slotType);


private:
	Array<EquipmentSlot, EQUIPMENT_TOTAL_SLOTS> _slots;
};

