#include "pch.h"
#include "Player.h"

#include "Room.h"

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
