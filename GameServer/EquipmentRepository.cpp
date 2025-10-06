#include "pch.h"
#include "EquipmentRepository.h"

// ===== EquipmentInstance 관련 함수 =====

EquipmentInstance EquipmentRepository::GetEquipmentInstance_DB(DBConnection& conn, int equipmentInstanceId)
{
	int outEquipmentInstanceId = 0;
	int itemId = 0;
	int enhancementLevel = 0;
	TIMESTAMP_STRUCT acquiredAt;

	SP::GetEquipmentInstance sp(conn);
	sp.ParamIn_EquipmentInstanceId(equipmentInstanceId);
	sp.ColumnOut_EquipmentInstanceId(OUT outEquipmentInstanceId);
	sp.ColumnOut_ItemId(OUT itemId);
	sp.ColumnOut_EnhancementLevel(OUT enhancementLevel);
	sp.ColumnOut_AcquiredAt(OUT acquiredAt);

	sp.Execute();

	if (sp.Fetch())
	{
		auto timePoint = std::chrono::system_clock::now(); // TODO: TIMESTAMP_STRUCT 변환
		EquipmentInstance instance(outEquipmentInstanceId, itemId, enhancementLevel, timePoint);

		GConsoleLogger->WriteStdOut(Color::GREEN,
			L"Loaded equipment instance - InstanceID[%d] ItemID[%d] Enhancement[%d]\n",
			outEquipmentInstanceId, itemId, enhancementLevel);

		return instance;
	}

	// 조회 실패 시 빈 인스턴스 반환
	GConsoleLogger->WriteStdOut(Color::RED,
		L"Failed to load equipment instance - InstanceID[%d]\n", equipmentInstanceId);

	return EquipmentInstance();
}

std::future<EquipmentInstance> EquipmentRepository::GetEquipmentInstanceAsync(int equipmentInstanceId)
{
	return DbDispatcher::EnqueueRet([equipmentInstanceId](DBConnection& c) {
		return GetEquipmentInstance_DB(c, equipmentInstanceId);
	});
}

int EquipmentRepository::CreateEquipmentInstance_DB(DBConnection& conn, int itemId, int enhancementLevel)
{
	int newEquipmentInstanceId = 0;

	SP::InsertEquipmentInstance sp(conn);
	sp.ParamIn_ItemId(itemId);
	sp.ParamIn_EnhancementLevel(enhancementLevel);
	sp.ParamOut_NewEquipmentInstanceId(OUT newEquipmentInstanceId);

	sp.Execute();

	GConsoleLogger->WriteStdOut(Color::GREEN,
		L"Created equipment instance - NewID[%d] ItemID[%d] Enhancement[%d]\n",
		newEquipmentInstanceId, itemId, enhancementLevel);

	return newEquipmentInstanceId;
}

std::future<int> EquipmentRepository::CreateEquipmentInstanceAsync(int itemId, int enhancementLevel)
{
	return DbDispatcher::EnqueueRet([itemId, enhancementLevel](DBConnection& c) {
		return CreateEquipmentInstance_DB(c, itemId, enhancementLevel);
	});
}

void EquipmentRepository::UpdateEquipmentInstance_DB(DBConnection& conn, int equipmentInstanceId, int enhancementLevel)
{
	SP::UpdateEquipmentInstance sp(conn);
	sp.ParamIn_EquipmentInstanceId(equipmentInstanceId);
	sp.ParamIn_EnhancementLevel(enhancementLevel);

	sp.Execute();

	GConsoleLogger->WriteStdOut(Color::BLUE,
		L"Updated equipment instance - InstanceID[%d] Enhancement[%d]\n",
		equipmentInstanceId, enhancementLevel);
}

std::future<void> EquipmentRepository::UpdateEquipmentInstanceAsync(int equipmentInstanceId, int enhancementLevel)
{
	return DbDispatcher::Enqueue([equipmentInstanceId, enhancementLevel](DBConnection& c) {
		UpdateEquipmentInstance_DB(c, equipmentInstanceId, enhancementLevel);
	});
}

void EquipmentRepository::DeleteEquipmentInstance_DB(DBConnection& conn, int equipmentInstanceId)
{
	SP::DeleteEquipmentInstance sp(conn);
	sp.ParamIn_EquipmentInstanceId(equipmentInstanceId);

	sp.Execute();

	GConsoleLogger->WriteStdOut(Color::YELLOW,
		L"Deleted equipment instance - InstanceID[%d]\n",
		equipmentInstanceId);
}

std::future<void> EquipmentRepository::DeleteEquipmentInstanceAsync(int equipmentInstanceId)
{
	return DbDispatcher::Enqueue([equipmentInstanceId](DBConnection& c) {
		DeleteEquipmentInstance_DB(c, equipmentInstanceId);
	});
}

// ===== CharacterEquipment 관련 함수 =====

Vector<EquipmentSlot> EquipmentRepository::GetCharacterEquipment_DB(DBConnection& conn, EntityId characterId)
{
	Vector<EquipmentSlot> slots;

	int slotType;
	int equipmentInstanceId;
	int itemId;
	int enhancementLevel;
	int out_characterId;

	SP::GetCharacterEquipment sp(conn);
	sp.ParamIn_CharacterId(static_cast<int>(characterId));
	sp.ColumnOut_CharacterId(OUT out_characterId);
	sp.ColumnOut_SlotType(OUT slotType);
	sp.ColumnOut_EquipmentInstanceId(OUT equipmentInstanceId);
	sp.ColumnOut_ItemId(OUT itemId);
	sp.ColumnOut_EnhancementLevel(OUT enhancementLevel);

	sp.Execute();

	while (sp.Fetch())
	{
		EquipmentSlot slot(static_cast<Protocol::EEquipmentSlotType>(slotType), equipmentInstanceId, itemId, enhancementLevel);
		slots.push_back(slot);

		GConsoleLogger->WriteStdOut(Color::GREEN,
			L"Loaded character equipment - CharID[%d] SlotType[%d] InstanceID[%d] ItemID[%d] EnhanceLevel[%d]\n",
			out_characterId, slotType, equipmentInstanceId, itemId, enhancementLevel);
	}

	return slots;
}

std::future<Vector<EquipmentSlot>> EquipmentRepository::GetCharacterEquipmentAsync(EntityId characterId)
{
	return DbDispatcher::EnqueueRet([characterId](DBConnection& c) {
		return GetCharacterEquipment_DB(c, characterId);
	});
}

void EquipmentRepository::UpsertCharacterEquipment_DB(DBConnection& conn, EntityId characterId, Protocol::EEquipmentSlotType slotType, int equipmentInstanceId)
{
	SP::UpsertCharacterEquipment sp(conn);
	sp.ParamIn_CharacterId(static_cast<int>(characterId));
	sp.ParamIn_SlotType(static_cast<int>(slotType));
	sp.ParamIn_EquipmentInstanceId(equipmentInstanceId);

	sp.Execute();

	GConsoleLogger->WriteStdOut(Color::BLUE,
		L"Upserted character equipment - CharID[%d] SlotType[%d] InstanceID[%d]\n",
		characterId, static_cast<int>(slotType), equipmentInstanceId);
}

std::future<void> EquipmentRepository::UpsertCharacterEquipmentAsync(EntityId characterId, Protocol::EEquipmentSlotType slotType, int equipmentInstanceId)
{
	return DbDispatcher::Enqueue([characterId, slotType, equipmentInstanceId](DBConnection& c) {
		UpsertCharacterEquipment_DB(c, characterId, slotType, equipmentInstanceId);
	});
}

void EquipmentRepository::DeleteCharacterEquipment_DB(DBConnection& conn, EntityId characterId, Protocol::EEquipmentSlotType slotType)
{
	SP::DeleteCharacterEquipment sp(conn);
	sp.ParamIn_CharacterId(static_cast<int>(characterId));
	sp.ParamIn_SlotType(static_cast<int>(slotType));

	sp.Execute();

	GConsoleLogger->WriteStdOut(Color::YELLOW,
		L"Deleted character equipment - CharID[%d] SlotType[%d]\n",
		characterId, static_cast<int>(slotType));
}

std::future<void> EquipmentRepository::DeleteCharacterEquipmentAsync(EntityId characterId, Protocol::EEquipmentSlotType slotType)
{
	return DbDispatcher::Enqueue([characterId, slotType](DBConnection& c) {
		DeleteCharacterEquipment_DB(c, characterId, slotType);
	});
}
