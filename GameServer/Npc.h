#pragma once
#include "EntityCore.h"
#include "NpcDialogComponent.h"
#include "NpcQuestComponent.h"
#include "NpcShopComponent.h"

// NPC 역할
enum class ENpcRole : uint8_t
{
	Dialog = 0,
	Shop = 1, 
	Quest = 2, 
	Mixed = 3 // Mixed Role, not Implemented yet.
};


// 현재 NPC의 state
enum class ENpcState : uint8_t
{
	Idle = 0,
	Talking = 1,
	Trading = 2
};

// NPC의 Interaction Type
enum class ENpcInteractionType : uint8_t
{
	Talk = 0,
	Shop = 1,
	Quest = 2
};

struct NpcConfig
{
	int				npcId {};
	ENpcRole		role {};
	std::string		name {};
	int				x{}, y{};
	int				dialogId {};
	int				shopId {};
	vector<int>		questIds {};
};


class Npc
{
	EntityCore	core; // 
	std::string name;
	ENpcRole	role;
	ENpcState	state;


public:
	void Initialize(const NpcConfig& cfg);
	void HandleInteraction(int playerId, ENpcInteractionType type);
	bool IsNearby(const Pos2& playerPos, int maxDistance = 1) const;

public:
	// Getter
	int NpcId() {return core.id;}
	const std::string& Name() {return name;}
	ENpcRole Role() {return role;}
	ENpcState State() {return state;}
	Pos2 Position() {return core.pos;}

	NpcDialogComponent* GetDialogComponent() { return _dialog.get(); }
	NpcShopComponent* GetShopComponent() { return _shop.get(); }
	NpcQuestComponent* GetQuestComponent() { return _quest.get(); }
private:
	std::unique_ptr<NpcDialogComponent> _dialog;
	std::unique_ptr<NpcShopComponent> _shop;
	std::unique_ptr<NpcQuestComponent> _quest;
};

