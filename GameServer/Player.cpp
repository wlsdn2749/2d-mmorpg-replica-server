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
	outStat.level = Level();
	outStat.exp = Exp();
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
