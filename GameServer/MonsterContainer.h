#pragma once
#include "MonsterTypes.h"

struct RespawnTask {
	int64_t dueMs{ 0 };
	SpawnPointCfg spawnCfg; // 사본 보관 (지점 설정)
};


class MonsterContainer {
public:
	Monster& CreateFromSpawn(const SpawnPointCfg& sp, const MonsterStats& stats,
		EntityId newId, int64_t nowMs);
	bool Exists(EntityId id) const { return _monsters.find(id) != _monsters.end(); }
	Monster& Get(EntityId id) { return _monsters.at(id); }
	const Monster* Find(EntityId id) const;


	void Kill(EntityId id); // 상태만 Dead로, 브로드캐스트/리스폰은 시스템에서
	bool Remove(EntityId id); // 실제 제거 (Despawn 이후)


	void EnqueueRespawn(int64_t dueMs, const SpawnPointCfg& sp);
	bool PeekRespawn(int64_t nowMs, RespawnTask& out) const;
	bool PopRespawn(int64_t nowMs, RespawnTask& out);


	// 스폰별 현재 살아있는 수 추적
	int AliveCountBySpawn(int spId) const;
	void IncAlive(int spId);
	void DecAlive(int spId);


	// 전체 순회 유틸
	template <typename Fn>
	void ForEachMonster(Fn&& fn) {
		for (auto& kv : _monsters) fn(kv.second);
	}

	template <typename Fn>
	void ForEachMonster(Fn&& fn) const {
		for(const auto& kv : _monsters) fn(kv.second);
	}

private:
	std::unordered_map<EntityId, Monster> _monsters;
	std::unordered_map<int, int> _aliveBySpawn; // spawnId -> alive


	// dueMs 오름차순
	struct Cmp { bool operator()(const RespawnTask& a, const RespawnTask& b) const { return a.dueMs > b.dueMs; } };
	std::priority_queue<RespawnTask, std::vector<RespawnTask>, Cmp> _respawns;
};