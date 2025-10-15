#include "pch.h"
#include "Npc.h"

void Npc::Initialize(const NpcConfig& cfg)
{
	core.id		= cfg.npcId;
	role		= cfg.role;
	name		= cfg.name;
	core.pos	= {cfg.x, cfg.y};
	
	if(cfg.shopId != 0)
		_shop		= std::make_unique<NpcShopComponent>(cfg.shopId);

	//_dialog = std::make_unique<NpcDialogComponent>(cfg.dialogId);
	//_quest = std::make_unique<NpcQuestComponent>(cfg.questIds);
}

const ENpcInteractionType Npc::GetAvailableType() const
{
	switch (role)
	{
	case ENpcRole::Dialog: 
		return ENpcInteractionType::Talk;
	case ENpcRole::Shop:
		return ENpcInteractionType::Shop;
	case ENpcRole::Quest:
		return ENpcInteractionType::Quest;
	}
}

void Npc::HandleInteraction(int playerId, ENpcInteractionType type)
{
	switch (type)
	{
	case ENpcInteractionType::Quest:
	case ENpcInteractionType::Talk:
		break;
	case ENpcInteractionType::Shop:
		//_shop->
		break;
	default:
		break;
	}
}

bool Npc::IsNearby(const Pos2& playerPos, int maxDistance) const
{
	return Manhattan(Position(), playerPos) <= maxDistance;
}

Protocol::S_NpcInteractReply Npc::GetBasicInfo() const
{
	Protocol::S_NpcInteractReply pkt;
	auto* dialogs = pkt.mutable_dialogs();
	dialogs->Add();
	
	auto type = GetAvailableType();
	pkt.set_interactiontype(static_cast<int>(type));
	return pkt;
}


