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
