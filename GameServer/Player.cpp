#include "pch.h"
#include "Player.h"

#include "Room.h"
#include "InventoryRepository.h"

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
	SetMaxHp(stat.maxHp);
	SetLevel(stat.level);
	SetExp(stat.exp);
	SetMoney(stat.money);

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
	auto fut = SaveInventoryToDB(); // 연결 종료시, DB 저장
	return fut;
}

std::unique_ptr<Protocol::PlayerStatInfo> Player::GetPlayerStatInfo() const
{
	auto infoPkt = std::make_unique<Protocol::PlayerStatInfo>();
	infoPkt->set_maxhp(MaxHp());
	infoPkt->set_hp(Hp());
	infoPkt->set_exp(Exp());
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
