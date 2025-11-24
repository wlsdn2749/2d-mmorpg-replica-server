#include "pch.h"
#include "EquipmentManager.h"
#include "EquipmentDataParser.h"
#include <fstream>

bool EquipmentManager::Initialize()
{
	if (_initialized)
		return true;

	GConsoleLogger->WriteStdOut(Color::YELLOW, L"EquipmentManager: Initializing...\n");

	// JSON에서 장비 데이터 로드 (move)
	_equipmentDataMap = EquipmentDataParser::LoadEquipmentData();

	if (_equipmentDataMap.size() == 0)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"EquipmentManager: Failed to load equipment data from JSON\n");
		return false;
	}

	_initialized = true;

	GConsoleLogger->WriteStdOut(Color::GREEN, L"EquipmentManager: Initialization complete. Total %d equipments loaded.\n",
		static_cast<int>(_equipmentDataMap.size()));

	return true;
}

void EquipmentManager::Shutdown()
{
	if (!_initialized)
		return;

	GConsoleLogger->WriteStdOut(Color::YELLOW, L"EquipmentManager: Shutting down...\n");

	_equipmentDataMap.clear();
	_initialized = false;

	GConsoleLogger->WriteStdOut(Color::GREEN, L"EquipmentManager: Shutdown complete.\n");
}

const EquipmentData* EquipmentManager::GetEquipmentData(int itemId) const
{
	auto it = _equipmentDataMap.find(itemId);
	return (it != _equipmentDataMap.end()) ? it->second.get() : nullptr;
}

bool EquipmentManager::IsEquipment(int itemId) const
{
	return _equipmentDataMap.find(itemId) != _equipmentDataMap.end();
}

Protocol::EEquipmentSlotType EquipmentManager::GetSlotType(int itemId) const
{
	const EquipmentData* data = GetEquipmentData(itemId);
	return data ? data->slotType : Protocol::EEquipmentSlotType::EQUIPMENT_WEAPON;
}

int EquipmentManager::GetMinLevel(int itemId) const
{
	const EquipmentData* data = GetEquipmentData(itemId);
	return data ? data->minLevel : 0;
}

int EquipmentManager::GetAtk(int itemId) const
{
	const EquipmentData* data = GetEquipmentData(itemId);
	return data ? data->atk : 0;
}

int EquipmentManager::GetDef(int itemId) const
{
	const EquipmentData* data = GetEquipmentData(itemId);
	return data ? data->def : 0;
}

int EquipmentManager::GetMaxHp(int itemId) const
{
	const EquipmentData* data = GetEquipmentData(itemId);
	return data ? data->maxHp : 0;
}

bool EquipmentManager::CanEquip(int itemId, int playerLevel) const
{
	const EquipmentData* data = GetEquipmentData(itemId);
	if (!data)
		return false;

	return playerLevel >= data->minLevel;
}

void EquipmentManager::AddEquipmentData(EquipmentData&& equipmentData)
{
	int itemId = equipmentData.itemId;
	_equipmentDataMap[itemId] = std::make_unique<EquipmentData>(std::move(equipmentData));
}

void EquipmentManager::RemoveEquipmentData(int itemId)
{
	_equipmentDataMap.erase(itemId);
}

void EquipmentManager::PrintAllEquipments() const
{
	GConsoleLogger->WriteStdOut(Color::GREEN, L"=== EquipmentManager: All Equipments ===\n");

	for (const auto& pair : _equipmentDataMap)
	{
		const EquipmentData* data = pair.second.get();
		GConsoleLogger->WriteStdOut(Color::WHITE,
			L"ItemID: %d, SlotType: %d, MinLevel: %d, Atk: %d, Def: %d, MaxHp: %d\n",
			data->itemId,
			static_cast<int>(data->slotType),
			data->minLevel,
			data->atk,
			data->def,
			data->maxHp);
	}

	GConsoleLogger->WriteStdOut(Color::GREEN, L"Total equipments: %d\n", static_cast<int>(_equipmentDataMap.size()));
}

size_t EquipmentManager::GetEquipmentCount() const
{
	return _equipmentDataMap.size();
}


// 편의를 위한 전역 접근 함수들
namespace EquipmentManagerGlobal
{
	const EquipmentData* GetEquipmentData(int itemId)
	{
		return EquipmentManager::Instance().GetEquipmentData(itemId);
	}

	bool IsEquipment(int itemId)
	{
		return EquipmentManager::Instance().IsEquipment(itemId);
	}

	Protocol::EEquipmentSlotType GetSlotType(int itemId)
	{
		return EquipmentManager::Instance().GetSlotType(itemId);
	}
}
