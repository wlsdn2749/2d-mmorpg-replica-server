#include "pch.h"
#include "Player.h"

#include "Room.h"
#include "InventoryRepository.h"
#include "EquipmentManager.h"

void Player::GetCharacterStat(CharacterRepository::CharacterStat& outStat) const
{
	outStat.characterId = static_cast<int>(core.id);
	outStat.posX = PosX();
	outStat.posY = PosY();
	outStat.dir = Dir();
	outStat.lastRoom = GetRoom()->RoomId();
	outStat.hp = Hp();
	outStat.maxHp = MaxHp();
	outStat.level = Level();
	outStat.exp = Exp();
	outStat.money = Money();
	GConsoleLogger->WriteStdOut(Color::WHITE, L"[GetCharacterStat] characterId:%d, Pos:(%d,%d), Dir:%d, lastRoom:%d, hp:%d, maxHp:%d, exp:%d, money:%d",
		outStat.characterId,
		outStat.posX,
		outStat.posY,
		outStat.dir,
		outStat.lastRoom,
		outStat.hp,
		outStat.maxHp,
		outStat.level,
		outStat.exp,
		outStat.money
	);
}

void Player::LoadCharacterStat(const CharacterRepository::CharacterStat& stat)
{
	core.id = static_cast<int>(stat.characterId);
	playerId = stat.characterId;
	core.pos.x = stat.posX;
	core.pos.y = stat.posY;
	core.dir = stat.dir;
	SetLastRoomId(stat.lastRoom);
	SetHp(stat.hp);
	SetLevel(stat.level);
	SetExp(stat.exp);
	SetMoney(stat.money);

	// DB의 스탯을 기본 스탯으로 설정 (장비 보너스 제외)
	// TODO: 나중에 DB 스키마에 base 스탯 컬럼 추가
	SetBaseMaxHp(stat.maxHp);
	SetBaseAtk(10); // 임시 기본값
	SetBaseDef(5);  // 임시 기본값

	// 일단 최종 스탯도 기본 스탯과 동일하게 설정 (장비 로드 후 RecalculateStats 호출됨)
	SetMaxHp(stat.maxHp);
	SetAtk(10);
	SetDef(5);

	GConsoleLogger->WriteStdOut(Color::YELLOW, L"[LoadCharacterStat] characterId:%d, Pos:(%d,%d), Dir:%d, lastRoom:%d, hp:%d, maxHp:%d, exp:%d, money:%d",
		stat.characterId,
		stat.posX,
		stat.posY,
		stat.dir,
		stat.lastRoom,
		stat.hp,
		stat.maxHp,
		stat.level,
		stat.exp,
		stat.money
	);
}

std::future<void> Player::SaveCharacterToDB()
{
	GConsoleLogger->WriteStdOut(Color::YELLOW, L"SaveCharacterToDB 실행");
	CharacterRepository::CharacterStat stat;
	GetCharacterStat(stat);
	CharacterRepository::UpdateCharacterStatsAsync(stat); // 연결종료 시, Stats 저장
	SaveInventoryToDB(); // 연결 종료시, 인벤토리 저장
	auto fut = SaveEquipmentToDB(); // 연결 종료시, 장비 저장
	return fut;
}

std::unique_ptr<Protocol::PlayerStatInfo> Player::GetPlayerStatInfo() const
{
	auto infoPkt = std::make_unique<Protocol::PlayerStatInfo>();
	infoPkt->set_maxhp(MaxHp());
	infoPkt->set_hp(Hp());
	infoPkt->set_curexp(Exp());
	infoPkt->set_maxexp(MaxExp());
	infoPkt->set_level(Level());
	infoPkt->set_money(Money());
	return infoPkt;
}

std::future<void> Player::LoadInventoryFromDB()
{
	int characterId = static_cast<int>(playerId);
	
	return DbDispatcher::Enqueue([this, characterId](DBConnection& c) {
		try {
			auto slots = InventoryRepository::GetCharacterInventory_DB(c, characterId);
			_inventory.FromInventorySlots(slots);
			
			GConsoleLogger->WriteStdOut(Color::GREEN,
				L"Player[%d]: Loaded %d inventory slots from DB\n", 
				characterId, static_cast<int>(slots.size()));
		}
		catch (const std::exception& e) {
			GConsoleLogger->WriteStdOut(Color::RED,
				L"Player[%d]: Failed to load inventory from DB: %S\n", 
				characterId, e.what());
		}
	});
}

std::future<void> Player::SaveInventoryToDB()
{
	int characterId = static_cast<int>(playerId);
	
	// 현재 인벤토리의 모든 슬롯을 가져오기
	std::vector<ItemSlot> slots;
	for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
	{
		const ItemSlot& slot = _inventory.GetSlot(i);
		if (!slot.IsEmpty())
		{
			slots.push_back(slot);
		}
	}
	
	return DbDispatcher::Enqueue([this, characterId, slots](DBConnection& c) {
		try {
			InventoryRepository::SaveFullInventory_DB(c, characterId, slots);
			
			GConsoleLogger->WriteStdOut(Color::GREEN,
				L"Player[%d]: Saved %d inventory slots to DB\n", 
				characterId, static_cast<int>(slots.size()));
		}
		catch (const std::exception& e) {
			GConsoleLogger->WriteStdOut(Color::RED,
				L"Player[%d]: Failed to save inventory to DB: %S\n",
				characterId, e.what());
		}
	});
}

std::future<void> Player::LoadEquipmentFromDB()
{
	EntityId characterId = playerId;

	return DbDispatcher::Enqueue([this, characterId](DBConnection& c) {
		try {
			auto slots = EquipmentRepository::GetCharacterEquipment_DB(c, characterId);

			// 장착된 장비 슬롯 로드
			for (const auto& slot : slots)
			{
				_equipment.EquipItem(slot.slotType, slot.itemId, slot.equipmentInstanceId, slot.enhancementLevel);
			}

			// 장비 스탯 재계산
			RecalculateStats();

			GConsoleLogger->WriteStdOut(Color::GREEN,
				L"Player[%d]: Loaded %d equipment slots from DB\n",
				characterId, static_cast<int>(slots.size()));
		}
		catch (const std::exception& e) {
			GConsoleLogger->WriteStdOut(Color::RED,
				L"Player[%d]: Failed to load equipment from DB: %S\n",
				characterId, e.what());
		}
	});
}

std::future<void> Player::SaveEquipmentToDB()
{
	EntityId characterId = playerId;

	// 현재 장착된 모든 장비 슬롯을 가져오기
	std::vector<EquipmentSlot> slots;
	for (int i = 0; i < EQUIPMENT_TOTAL_SLOTS; ++i)
	{
		Protocol::EEquipmentSlotType slotType = static_cast<Protocol::EEquipmentSlotType>(i);
		const EquipmentSlot& slot = _equipment.GetEquipmentSlot(slotType);
		if (!slot.IsEmpty())
		{
			slots.push_back(slot);
		}
	}

	return DbDispatcher::Enqueue([this, characterId, slots](DBConnection& c) {
		try {
			// 각 슬롯을 DB에 저장
			for (const auto& slot : slots)
			{
				EquipmentRepository::UpsertCharacterEquipment_DB(c, characterId, slot.slotType, slot.equipmentInstanceId);
			}

			GConsoleLogger->WriteStdOut(Color::GREEN,
				L"Player[%d]: Saved %d equipment slots to DB\n",
				characterId, static_cast<int>(slots.size()));
		}
		catch (const std::exception& e) {
			GConsoleLogger->WriteStdOut(Color::RED,
				L"Player[%d]: Failed to save equipment to DB: %S\n",
				characterId, e.what());
		}
	});
}

void Player::RecalculateStats()
{
	// 장비 보너스 계산
	int equipAtk = 0;
	int equipDef = 0;
	int equipMaxHp = 0;

	for (int i = 0; i < EQUIPMENT_TOTAL_SLOTS; ++i)
	{
		Protocol::EEquipmentSlotType slotType = static_cast<Protocol::EEquipmentSlotType>(i);
		const EquipmentSlot& slot = _equipment.GetEquipmentSlot(slotType);

		if (!slot.IsEmpty())
		{
			const EquipmentData* data = EquipmentManager::Instance().GetEquipmentData(slot.itemId);
			if (data)
			{
				// 기본 스탯 + 강화 보너스 (강화당 10% 증가)
				float enhancementMultiplier = 1.0f + (slot.enhancementLevel * 0.1f);
				equipAtk += static_cast<int>(data->atk * enhancementMultiplier);
				equipDef += static_cast<int>(data->def * enhancementMultiplier);
				equipMaxHp += static_cast<int>(data->maxHp * enhancementMultiplier);
			}
		}
	}

	// 최종 스탯 = 기본 스탯 + 장비 보너스
	_atk = _baseAtk + equipAtk;
	_def = _baseDef + equipDef;
	_maxHp = _baseMaxHp + equipMaxHp;

	// 현재 HP가 최대 HP를 초과하지 않도록 조정
	if (_hp > _maxHp)
	{
		_hp = _maxHp;
	}

	GConsoleLogger->WriteStdOut(Color::WHITE,
		L"[RecalculateStats] Player[%d]: Base(Atk:%d, Def:%d, MaxHp:%d) + Equip(Atk:%d, Def:%d, MaxHp:%d) = Final(Atk:%d, Def:%d, MaxHp:%d)\n",
		static_cast<int>(playerId),
		_baseAtk, _baseDef, _baseMaxHp,
		equipAtk, equipDef, equipMaxHp,
		_atk, _def, _maxHp
	);
}

