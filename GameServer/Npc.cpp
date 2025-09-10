#include "pch.h"
#include "Npc.h"

void Npc::Initialize(const NpcConfig& cfg)
{
	core.id		= cfg.npcId;
	role		= cfg.role;
	name		= cfg.name;
	core.pos	= {cfg.x, cfg.y};
	// dialogId -> unique_ptr<?>
	// shopId
	// questIds
}

void Npc::HandleInteraction(int playerId, ENpcInteractionType type)
{
	;
}

bool Npc::IsNearby(const Pos2& playerPos, int maxDistance) const
{
	// TODO implementation
	return false; 
}
