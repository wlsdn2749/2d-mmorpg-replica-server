#pragma once
#include "MonsterContainer.h"
#include "MonsterPorts.h"
#include "MonsterSpawnerSystem.h"

class MonsterCombatSystem
{
public:
	void Tick(MonsterContainer& repo, const MonsterSpawnerSystem& spawner,
		IMonsterEntityLinker& linker, IMonsterBroadcaster& cast, IMonsterClock& clock,
		const std::unordered_map<int, MonsterStats>& statsByType);


private:
	void TickOne(Monster& m, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast, IMonsterClock& clock,
		const MonsterStats& stats);

	void setTarget(Monster&m, IMonsterEntityLinker& linker, const MonsterStats& stats);
	void ExecuteAttack(Monster& m, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast, IMonsterClock& clock, const MonsterStats& stats);

private:
	// Helper
	bool isValidTarget(Monster& m) {return m.targetPlayerId != -1; }
	bool isReadyState(Monster& m) {return m.state == MState::Ready; }
	bool inAttackRange(int distance, const MonsterStats& stats) {return distance <= stats.attackRangeTiles;}

};


