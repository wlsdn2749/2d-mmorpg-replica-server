#pragma once
#include "MonsterContainer.h"
#include "MonsterPorts.h"

class MonsterSpawnerSystem {
public:
	void Init(MonsterContainer& repo, const std::vector<SpawnPointCfg>& spawns,
		const std::unordered_map<int, MonsterStats>& statsByType,
		IMonsterBroadcaster& cast, IMonsterClock& clock);


	void Tick(MonsterContainer& repo, IMonsterBroadcaster& cast, IMonsterClock& clock);


	const std::vector<SpawnPointCfg>& Spawns() const { return _spawns; }
	const MonsterStats& GetStats(int typeId) const { return _statsByType.at(typeId); }


private:
	std::vector<SpawnPointCfg> _spawns;
	std::unordered_map<int, MonsterStats> _statsByType; // typeId -> stats
	EntityId _idSeq{ 1 };
};