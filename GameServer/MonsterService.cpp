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
	_cast.DespawnMonster(id, Protocol::EDespawnReason::DESPAWN_UNKNOWN);


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

/* 읽기 뷰 (퍼사드) */
void MonsterService::ForEachMonsterView(const std::function<void(const MonsterView&)>& fn) const
{
	_container.ForEachMonster([&](const Monster& m)
		{
			fn(MonsterView{ m.core.id, m.core.pos.x, m.core.pos.y, m.curHp, m.typeId });
		});
}

bool MonsterService::TryGetMonsterView(EntityId id, MonsterView& out) const
{
	if (const Monster* m = _container.Find(id))
	{
		out = MonsterView{ m->core.id, m->core.pos.x, m->core.pos.y, m->curHp, m->typeId };
		return true;
	}
	return false;
}

/* 쓰기 (퍼사드) */

bool MonsterService::ApplyDamageToMonster(EntityId id, int dmg, int /*srcPlayerId*/, int& hpAfter)
{
	if (dmg <= 0) return false;

	Monster* m = _container.Find(id);
	if (!m) return false;

	hpAfter -= dmg;
	m->curHp = hpAfter;
	if(m->curHp > 0)
		return true;

	// HP가 0이하면 사망처리
	KillMonster(id, Protocol::EDespawnReason::DESPAWN_KILLED);
	return true;
}

void MonsterService::KillMonster(EntityId id, Protocol::EDespawnReason reason)
{
	Monster* m = _container.Find(id);
	if(!m) return;

	const int spawnPointId = m->fromSpawnId; // 스폰 포인트 id

	// 1) 디스폰 브로드 캐스트
	_cast.DespawnMonster(id, reason);

	// 2) 컨테이너에서 제거
	_container.Remove(id);
	// 
	// 3) 리스폰 예약
	_spawner.OnMonsterDeath(_container, spawnPointId, _clock);

	// 4) 죽음 브로드캐스트
	_cast.BroadcastMonsterDeath(id);
}