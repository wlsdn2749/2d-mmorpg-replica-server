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
			  IMonsterEntityLinker& linker,
			  IMonsterBroadcaster& cast,
			  IMonsterClock& clock,
			  IMonsterRng& rng);


private:
	Cfg _cfg;

	// 메인 처리
	void TickOne(Monster& m, 
				 const MonsterSpawnerSystem& spawner,
				 IMonsterMapQuery& map,
				 IMonsterEntityLinker& linker,
				 IMonsterBroadcaster& cast,
				 IMonsterClock& clock,
				 IMonsterRng& rng);
	
	// 상태 전환 체크
	void CheckStateTransitions(Monster& m, 
							  const MonsterSpawnerSystem& spawner,
							  IMonsterEntityLinker& linker, 
							  IMonsterClock& clock,
							  const MonsterStats& stats);
	
	// 상태별 실행 로직
	void ExecutePatrolState(Monster& m, IMonsterMapQuery& map, IMonsterBroadcaster& cast, 
						   IMonsterClock& clock, IMonsterRng& rng, const MonsterStats& stats);
	void ExecuteReadyState(Monster& m, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast,
						  IMonsterClock& clock, const MonsterStats& stats);
	void ExecuteCombatState(Monster& m, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast,
						   IMonsterClock& clock, const MonsterStats& stats);
	void ExecuteChaseState(Monster& m, IMonsterEntityLinker& linker, IMonsterMapQuery& map,
						  IMonsterBroadcaster& cast, IMonsterClock& clock, const MonsterStats& stats);
	void ExecuteReturnState(Monster& m, IMonsterMapQuery& map, IMonsterBroadcaster& cast,
						   IMonsterClock& clock, const MonsterStats& stats);
	
	// 유틸리티 함수들
	int GetDistanceToTarget(Monster& m, IMonsterEntityLinker& linker);
	bool IsPlayerInAttackRange(Monster& m, IMonsterEntityLinker& linker, int attackRange);
	bool IsOutOfLeash(Monster& m, const MonsterSpawnerSystem& spawner);
	
	// 기존 이동 유틸
	bool TryRotate(Monster& m, Protocol::EDirection targetDir, IMonsterBroadcaster& cast);
	bool TryStep(Monster& m, Protocol::EDirection dir, IMonsterMapQuery& map, IMonsterBroadcaster& cast);
};