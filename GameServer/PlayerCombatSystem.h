#pragma once
#include "PlayerPorts.h"

class PlayerCombatSystem
{
public:
	struct Cfg {
		int attackCooldownMs = 500;
	};

	PlayerCombatSystem(const Cfg& cfg, IPlayerMonsterLinker& pLinker, IPlayerCombatBroadcaster& pCaster)
		: _cfg(cfg), _pLinker(pLinker), _pCaster(pCaster)
	{ 
	}

	void HandleAttack(const PlayerRef& player, int64 nowMs);



private:
	// 바로 앞 타일의 MonsterView 구하기
	bool InFrontOneTile(const PlayerRef& player, const MonsterView& m) const;
	// 플레이어  공격 쿨타임 확인
	bool AttackCooldownReady(int pid, int64 nowMs) const;

private:
	Cfg _cfg;
	IPlayerMonsterLinker& _pLinker;
	IPlayerCombatBroadcaster& _pCaster;

	mutable std::unordered_map<int /*playerId*/, int64 /*lastAttackMs*/> _lastAttackAtMs;
};

