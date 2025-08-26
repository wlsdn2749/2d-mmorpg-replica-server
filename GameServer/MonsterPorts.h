#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include "TypeCore.h" // EntityId, EntityKind
#include "GeometryCore.h" // Pos2, Dir

// fwd
struct Monster;


// ===== Room이 제공해야 하는 포트 =====
struct IMonsterMapQuery {
	virtual ~IMonsterMapQuery() = default;
	virtual bool CanEnter(int x, int y) const = 0;
};


struct IMonsterEntityLinker {
	struct PlayerView { int id; int x; int y; int hp; };


	virtual ~IMonsterEntityLinker() = default;
	virtual void ForEachPlayerInRange(int cx, int cy, int rangeTiles,
		std::function<void(const PlayerView&)>) const = 0;
	virtual bool TryGetPlayer(int playerId, PlayerView& out) const = 0;


	// 쓰기 커맨드 (플레이어는 Room이 소유이므로 직접 수정 금지)
	virtual void ApplyDamageToPlayer(int playerId, int damage, int sourceMonsterId) = 0;
};


struct IMonsterBroadcaster {
	virtual ~IMonsterBroadcaster() = default;
	virtual void SpawnMonster(const Monster& m) = 0;
	virtual void DespawnMonster(EntityId id, Protocol::EDespawnReason reason) = 0; // 0:Dead, 1:Manual, ...
	virtual void BroadcastMonsterMove(EntityId id, int x, int y, Protocol::EDirection dir) = 0;
	virtual void BroadcastMonsterAttack(EntityId id, int targetPid) = 0;
	virtual void BroadcastMonsterDeath(EntityId id) = 0;
};


struct IMonsterClock {
	virtual ~IMonsterClock() = default;
	virtual int64_t NowMs() const = 0;
};


struct IMonsterRng {
	virtual ~IMonsterRng() = default;
	virtual int NextInt(int minIncl, int maxIncl) = 0; // [min, max]
};