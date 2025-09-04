#pragma once
#include <unordered_map>
#include "MonsterPorts.h"
#include "MonsterContainer.h"
#include "MonsterSpawnerSystem.h"
#include "MonsterMovementSystem.h"
#include "MonsterCombatSystem.h"


class MonsterService {
public:
	struct Cfg {
		std::vector<SpawnPointCfg> spawns;
		std::unordered_map<int, MonsterStats> statsByType; // typeId -> stats
		MonsterMovementSystem::Cfg movementCfg{};
	};


	MonsterService(IMonsterMapQuery& map, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast,
		IMonsterClock& clock, IMonsterRng& rng)
		: _map(map), _linker(linker), _cast(cast), _clock(clock), _rng(rng), _move(_moveCfg) 
	{
	}


	void Init(const Cfg& cfg);
	void Tick(int64_t /*deltaMs*/);


	// (옵션) 외부 이벤트 훅 
	void OnMonsterExternalKill(EntityId id);


	// 디버그/모니터링
	size_t AliveCount() const { size_t n = 0; _container.ForEachMonster([&](const Monster&) { ++n; }); return n; }

public:
	// 읽기 뷰
	struct MonsterView { EntityId id; int x, y, hp, typeId; }; // 서비스 내부에서 사용
	void ForEachMonsterView(const std::function<void(const MonsterView&)>& fn) const;
	bool TryGetMonsterView(EntityId id, MonsterView& out) const;

	// 쓰기
	bool ApplyDamageToMonster(EntityId id, int dmg, int srcPlayerId, int& hpAfter); // hp↓, 0이하면 KillMonster(reason=Dead)
	// 사망 처리(디스폰+리스폰 예약+Death 브로드캐스트)
	void KillMonster(EntityId id, Protocol::EDespawnReason reason);

private:
	// 외부 포트
	IMonsterMapQuery& _map;
	IMonsterEntityLinker& _linker;
	IMonsterBroadcaster& _cast;
	IMonsterClock& _clock;
	IMonsterRng& _rng;


	// 내부 시스템
	MonsterContainer _container;
	MonsterSpawnerSystem _spawner;
	MonsterMovementSystem::Cfg _moveCfg{};
	MonsterMovementSystem _move{ _moveCfg };
	MonsterCombatSystem _combat;


	// 설정 캐시
	std::unordered_map<int, MonsterStats> _statsByType;
};