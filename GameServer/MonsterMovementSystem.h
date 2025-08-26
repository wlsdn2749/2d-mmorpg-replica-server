#pragma once
#include "MonsterContainer.h"
#include "MonsterPorts.h"
#include "MonsterSpawnerSystem.h"

class MonsterMovementSystem {
public:
	struct Cfg {
		int patrolMinMs{ 800 };
		int patrolMaxMs{ 1600 };
		Cfg() = default;
		Cfg(int minMs, int maxMs) : patrolMinMs(minMs), patrolMaxMs(maxMs) {}
	};


	explicit MonsterMovementSystem(const Cfg& cfg = {}) : _cfg(cfg) {}


	void Tick(MonsterContainer& repo,
			  const MonsterSpawnerSystem& spawner,
			  IMonsterMapQuery& map,
			  IMonsterBroadcaster& cast,
			  IMonsterClock& clock,
			  IMonsterRng& rng);


private:
	Cfg _cfg;
	// 내부 유틸
	void TickOne(Monster& m, 
				 const MonsterSpawnerSystem& spawner,
				 IMonsterMapQuery& map,
				 IMonsterBroadcaster& cast,
				 IMonsterClock& clock,
				 IMonsterRng& rng);
	
	bool TryStep(Monster& m, Protocol::EDirection dir, IMonsterMapQuery& map, IMonsterBroadcaster& cast);
};