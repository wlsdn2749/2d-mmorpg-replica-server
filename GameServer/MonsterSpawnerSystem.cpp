#include "pch.h"
#include "MonsterSpawnerSystem.h"

void MonsterSpawnerSystem::Init(MonsterContainer& repo, const std::vector<SpawnPointCfg>& spawns,
	const std::unordered_map<int, MonsterStats>& statsByType,
	IMonsterBroadcaster& cast, IMonsterClock& clock) 
{
	_spawns = spawns;
	_statsByType = statsByType;


	// 초기 스폰
	const int64_t now = clock.NowMs();
	for (const auto& sp : _spawns) {
		auto it = _statsByType.find(sp.monsterTypeId);
		if (it == _statsByType.end()) continue;
		const MonsterStats& stats = it->second;


		const int toSpawn = std::min(sp.initialSpawn, sp.maxAlive);
		for (int i = 0; i < toSpawn; ++i) {
			Monster& m = repo.CreateFromSpawn(sp, stats, _idSeq++, now);
			cast.SpawnMonster(m);
		}
	}
}


void MonsterSpawnerSystem::Tick(MonsterContainer& repo, IMonsterBroadcaster& cast, IMonsterClock& clock) {
	const int64_t now = clock.NowMs();


	// 리스폰 due 처리
	RespawnTask task;
	while (repo.PopRespawn(now, task)) {
		auto it = _statsByType.find(task.spawnCfg.monsterTypeId);
		if (it == _statsByType.end()) continue;
		// maxAlive 유지
		if (repo.AliveCountBySpawn(task.spawnCfg.id) >= task.spawnCfg.maxAlive) {
			// 자리 없으면 약간 지연 후 재큐(혼잡 해소)
			repo.EnqueueRespawn(now + 500, task.spawnCfg);
			continue;
		}
		Monster& m = repo.CreateFromSpawn(task.spawnCfg, it->second, _idSeq++, now);
		cast.SpawnMonster(m);
	}
}

void MonsterSpawnerSystem::OnMonsterDeath(MonsterContainer& repo, int spawnPointId, IMonsterClock& clock)
{
	if (const SpawnPointCfg* sp = FindSpawn(spawnPointId))
	{
		const int64_t when = clock.NowMs() + sp->respawnDelayMs;
		repo.EnqueueRespawn(when, *sp);
	}
}
