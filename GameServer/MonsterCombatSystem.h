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
};


