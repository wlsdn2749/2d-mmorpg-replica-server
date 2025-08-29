#pragma once
#include <functional>
#include "TypeCore.h"
#include "MonsterService.h"

using MonsterView = MonsterService::MonsterView;

// ===== Room이 제공해야하는 포트 ====
struct IPlayerMonsterLinker {
	
	virtual ~IPlayerMonsterLinker() = default;
	
	// 범위 내 몬스터 정보
	virtual void ForEachMonsterInRange(int cx, int cy, int rangeTiles, std::function<void(const MonsterView&)>) const = 0; 
	virtual bool TryGetMonster(EntityId monsterId, MonsterView& out) const = 0;
	virtual bool ApplyDamageToMonster(int monsterId, int damage, int srcPlayerId) = 0;

};

struct IPlayerCombatBroadcaster {
	virtual ~IPlayerCombatBroadcaster() = default;
	virtual void BroadcastPlayerAttack(int attackerId, int targetId, int damage, int hpAfter) = 0; // PlayerAttack Broadcast
	
};