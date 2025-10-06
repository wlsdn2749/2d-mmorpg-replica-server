#pragma once
#include "EquipmentCore.h"
#include "DBDisPatcher.h"
#include "GenProcedures.h"
#include <future>
#include <vector>

/*-------------------------
	장비 시스템 DB 진입점
-------------------------*/

// DB와 소통하는 장비 시스템의 진입점을 나타냅니다.
// 장비 장착, 해제, 교체 시 발생하는 DB Implement의 DB 레벨 Read/Write를 정의합니다.
// 기본적으로 SRP가 원칙이지만, 예외적으로 이 클래스는 EquipmentInstance와 CharacterEquipment에의 연결점을 작성합니다.

struct EquipmentRepository
{
	// 장비 상세 정보 조회(강화, 획득정보 등)
	static EquipmentInstance GetEquipmentInstance_DB(DBConnection& conn, int equipmentInstanceId);
	static std::future<EquipmentInstance> GetEquipmentInstanceAsync(int equipmentInstanceId);

	// 장비 생성
	static int CreateEquipmentInstance_DB(DBConnection& conn, int itemId, int enhancementLevel);
	static std::future<int> CreateEquipmentInstanceAsync(int itemId, int enhancementLevel);

	// 장비 업데이트
	static void UpdateEquipmentInstance_DB(DBConnection& conn, int equipmentInstanceId, int enhancementLevel);
	static std::future<void> UpdateEquipmentInstanceAsync(int equipmentInstanceId, int enhancementLevel);

	// 장비 삭제
	static void DeleteEquipmentInstance_DB(DBConnection& conn, int equipmentInstanceId);
	static std::future<void> DeleteEquipmentInstanceAsync(int equipmentInstanceId);

	// 캐릭터 장착 장비 정보 조회
	static Vector<EquipmentSlot> GetCharacterEquipment_DB(DBConnection& conn, EntityId characterId);
	static std::future<Vector<EquipmentSlot>> GetCharacterEquipmentAsync(EntityId characterId);

	// 캐릭터 장착 정보 Upsert
	static void UpsertCharacterEquipment_DB(DBConnection& conn, EntityId characterId, Protocol::EEquipmentSlotType slotType, int equipmentInstanceId);
	static std::future<void> UpsertCharacterEquipmentAsync(EntityId characterId, Protocol::EEquipmentSlotType slotType, int equipmentInstanceId);

	// 캐릭터 장착 정보 삭제
	static void DeleteCharacterEquipment_DB(DBConnection& conn, EntityId characterId, Protocol::EEquipmentSlotType slotType);
	static std::future<void> DeleteCharacterEquipmentAsync(EntityId characterId, Protocol::EEquipmentSlotType slotType);
};

