#include "pch.h"
#include "MonsterCombatSystem.h"

static inline int Mdist(const Pos2& a, const Pos2& b) {
	return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}


void MonsterCombatSystem::Tick(MonsterContainer& repo, const MonsterSpawnerSystem& spawner,
	IMonsterEntityLinker& linker, IMonsterBroadcaster& cast, IMonsterClock& clock,
	const std::unordered_map<int, MonsterStats>& statsByType) 
{
	const int64_t now = clock.NowMs();
	repo.ForEachMonster([&](Monster& m) {
		if (m.state == MState::Dead) // 현재 repo의 몬스터가 죽은 상태면?
			return;

		auto it = statsByType.find(m.typeId);
		if (it == statsByType.end()) 
			return;
		TickOne(m, linker, cast, clock, it->second);
		});
}


void MonsterCombatSystem::TickOne(Monster& m, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast, IMonsterClock& clock,
	const MonsterStats& stats) 
{
	// 타깃 선정(가장 가까운 플레이어 1명)
	int bestId = -1, bestDist = std::numeric_limits<int>::max();
	linker.ForEachPlayerInRange(m.core.pos.x, m.core.pos.y, stats.aggroRangeTiles,
		[&](const IMonsterEntityLinker::PlayerView& pv) {
			int d = std::abs(pv.x - m.core.pos.x) + std::abs(pv.y - m.core.pos.y);
			if (d < bestDist) { bestDist = d; bestId = pv.id; }
		});


	if (bestId < 0) {
		// 타깃 없음 → Idle/Patrol 유지 또는 Return은 Movement가 처리
		m.targetPlayerId = -1;
		return;
	}


	m.targetPlayerId = bestId;


	// 공격 시도
	const int64_t now = clock.NowMs();
	if (now >= m.nextAttackAtMs) {
		IMonsterEntityLinker::PlayerView pv;
		if (linker.TryGetPlayer(bestId, pv)) {
			int dist = Mdist(m.core.pos, Pos2{ pv.x, pv.y });
			if (dist <= stats.attackRangeTiles) {
				linker.ApplyDamageToPlayer(bestId, stats.atk, (int)m.core.id);
				cast.BroadcastMonsterAttack(m.core.id, bestId);
				m.nextAttackAtMs = now + stats.attackCooldownMs;
			}
		}
	}
}