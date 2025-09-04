#pragma once
#include "EntityCore.h"
#include <cstdint>
enum class MState : uint8_t { Idle, Patrol, Chase, Return, Dead };

/* 몬스터 별로 가지는 특성 */
struct MonsterStats {
	int32_t maxHp{ 20 };
	int32_t atk{ 3 };
	int32_t moveSpeedTilesPerSec{ 1 };
	int32_t attackRangeTiles{ 1 };
	int32_t attackCooldownMs{ 1200 };
	int32_t aggroRangeTiles{ 6 };

	MonsterStats() = default;
	MonsterStats(int32_t hp, int32_t a, int32_t spd, int32_t range, int32_t cd, int32_t aggro)
		: maxHp(hp), atk(a), moveSpeedTilesPerSec(spd),
		attackRangeTiles(range), attackCooldownMs(cd), aggroRangeTiles(aggro) {
	}
};


/* 어떤 몬스터가 어디 스폰할지? */
struct SpawnPointCfg {
	int32_t id{ 0 };
	int32_t x{ 0 }, y{ 0 };
	int32_t maxAlive{ 3 };
	int32_t initialSpawn{ 2 };
	int32_t respawnDelayMs{ 8000 };
	int32_t leashRadiusTiles{ 10 };
	int32_t monsterTypeId{ 1001 };

	SpawnPointCfg() = default;
	SpawnPointCfg(int32_t id_, int32_t x_, int32_t y_, int32_t maxAlive_,
		int32_t initialSpawn_, int32_t respawnDelayMs_,
		int32_t leashRadiusTiles_, int32_t monsterTypeId_)
		: id(id_), x(x_), y(y_), maxAlive(maxAlive_), initialSpawn(initialSpawn_),
		respawnDelayMs(respawnDelayMs_), leashRadiusTiles(leashRadiusTiles_), monsterTypeId(monsterTypeId_) {
	}
};


/* 몬스터 */
struct Monster {
	EntityCore core; // id/kind=Monster, pos/dir
	int32_t typeId{ 0 };
	int32_t curHp{ 1 };
	MState state{ MState::Idle };


	// 스폰/리쉬/타깃
	int32_t spawnX{ 0 }, spawnY{ 0 };
	int32_t fromSpawnId{ 0 };
	int32_t targetPlayerId{ -1 };


	// 타이밍
	int64_t nextMoveAtMs{ 0 };
	int64_t nextAttackAtMs{ 0 };
	int64_t lostAggroAtMs{ 0 };
};