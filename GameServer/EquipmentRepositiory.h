#pragma once

/*-------------------------
	장비 시스템 DB 진입점
-------------------------*/

// DB와 소통하는 장비 시스템의 진입점을 나타냅니다.
// 장비 장착, 해제, 교체 시 발생하는 DB Implement의 DB 레벨 Read/Write를 정의합니다.
// 기본적으로 SRP가 원칙이지만, 예외적으로 이 클래스는 EquipmentInstance와 CharacterEquipment에의 연결점을 작성합니다.

struct EquipmentRepositiory
{
	//static Vector<EquipmentInfo> GetEquipments_DB(DBConnection& conn, EntityId characterId);
	//static std::future<Vector<EquipmentInfo>> GetEquipmentsItemAsync(EntityId characterId);
};

