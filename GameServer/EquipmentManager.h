#pragma once

#include "Singleton.h"
#include "EquipmentCore.h"
#include <unordered_map>
#include <memory>

class EquipmentManager : public Singleton<EquipmentManager>
{

public:
	// 초기화 및 정리
	bool Initialize();
	void Shutdown();

	// 장비 데이터 조회
	const EquipmentData* GetEquipmentData(int itemId) const;
	bool IsEquipment(int itemId) const;
	Protocol::EEquipmentSlotType GetSlotType(int itemId) const;
	int GetMinLevel(int itemId) const;
	int GetAtk(int itemId) const;
	int GetDef(int itemId) const;
	int GetMaxHp(int itemId) const;

	// 장비 검증
	bool CanEquip(int itemId, int playerLevel) const;

	// 디버그 및 관리
	void AddEquipmentData(EquipmentData&& equipmentData);
	void RemoveEquipmentData(int itemId);
	void PrintAllEquipments() const;
	size_t GetEquipmentCount() const;

private:
	std::unordered_map<int, std::unique_ptr<EquipmentData>> _equipmentDataMap;
	bool _initialized = false;
};

// 편의를 위한 전역 접근 함수들
namespace EquipmentManagerGlobal
{
	const EquipmentData* GetEquipmentData(int itemId);
	bool IsEquipment(int itemId);
	Protocol::EEquipmentSlotType GetSlotType(int itemId);
}
