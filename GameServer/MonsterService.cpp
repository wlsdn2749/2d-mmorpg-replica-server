#include "pch.h"
#include "MonsterService.h"


void MonsterService::Init(const Cfg& cfg) {
	_statsByType = cfg.statsByType;
	_moveCfg = cfg.movementCfg;


	_spawner.Init(_container, cfg.spawns, _statsByType, _cast, _clock);
}


void MonsterService::Tick(int64_t /*deltaMs*/) {
	// 리스폰 처리
	_spawner.Tick(_container, _cast, _clock);


	// 이동 처리
	_move.Tick(_container, _spawner, _map, _cast, _clock, _rng);


	// 전투 처리
	_combat.Tick(_container, _spawner, _linker, _cast, _clock, _statsByType);
}


void MonsterService::OnMonsterExternalKill(EntityId id) {
	if (!_container.Exists(id)) return;
	auto& m = _container.Get(id);
	if (m.state == MState::Dead) return;
	_container.Kill(id);
	_cast.BroadcastMonsterDeath(id);
	_cast.DespawnMonster(id, Protocol::EDespawnReason::UNKNOWN);


	// 리스폰 예약
	// 해당 스폰 포인트 설정을 찾아 respawnDelayMs로 예약
	// (간단 구현: 스폰 초기 설정을 순회)
	for (const auto& sp : _spawner.Spawns()) {
		if (sp.id == m.fromSpawnId) {
			_container.EnqueueRespawn(_clock.NowMs() + sp.respawnDelayMs, sp);
			break;
		}
	}
	_container.Remove(id);
}