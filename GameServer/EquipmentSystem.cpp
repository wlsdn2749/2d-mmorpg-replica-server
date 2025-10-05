#include "pch.h"
#include "EquipmentSystem.h"

EquipmentSystem::EquipmentSystem()
{
	Initialize();
}

void EquipmentSystem::Initialize()
{
	Clear();
}

void EquipmentSystem::Clear()
{
	for (auto& slot : _slots)
	{
		slot.Clear();
	}
}

EEquipItemResult EquipmentSystem::EquipItem(Protocol::EEquipmentSlotType slotType, int equipmentInstanceId)
{
	int slotIndex = static_cast<int>(slotType);

	// 이미 장착된 장비 있으면 자동 해제
	if (!IsSlotEmpty(slotType))
	{
		UnequipItem(slotType);
	}

	// 새 장비 장착
	_slots[slotIndex].slotType = slotType;
	_slots[slotIndex].equipmentInstanceId = equipmentInstanceId;

	return EEquipItemResult::Success;
 }

void EquipmentSystem::UnequipItem(Protocol::EEquipmentSlotType slotType)
{
	int slotIndex = static_cast<int>(slotType);

	_slots[slotIndex].Clear();
}

bool EquipmentSystem::IsSlotEmpty(Protocol::EEquipmentSlotType slotType) const
{
	return GetEquipmentSlot(slotType).IsEmpty();
}

const EquipmentSlot& EquipmentSystem::GetEquipmentSlot(Protocol::EEquipmentSlotType slotType) const
{
	// TODO 유효성 검사
	return _slots[static_cast<int>(slotType)];
}

EquipmentSlot EquipmentSystem::GetEquipmentSlot(Protocol::EEquipmentSlotType slotType)
{
	// TODO 유효성 검사
	return _slots[static_cast<int>(slotType)];
}
