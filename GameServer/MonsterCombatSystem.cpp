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
	// 타깃 선정(가장 aggroRangeTiles 안에 있는 가장 가까운 플레이어 1명)
	setTarget(m, linker, stats);

	// Get Player
	IMonsterEntityLinker::PlayerView pv;
	if(linker.TryGetPlayer(m.targetPlayerId, pv) == false) return;

	// 공격 시도
	if (m.wasAttacked && m.state == MState::Combat)
	{
		ExecuteAttack(m, linker, cast, clock, stats);
	}


}

void MonsterCombatSystem::setTarget(Monster& m, IMonsterEntityLinker& linker, const MonsterStats& stats)
{
	if(isValidTarget(m)) return; // targetPlayerId가 없을 경우만 체크

	int bestId = -1, bestDist = std::numeric_limits<int>::max();
	linker.ForEachPlayerInRange(m.core.pos.x, m.core.pos.y, stats.aggroRangeTiles,
		[&](const IMonsterEntityLinker::PlayerView& pv) 
		{
			int d = std::abs(pv.x - m.core.pos.x) + std::abs(pv.y - m.core.pos.y);
			if (d < bestDist) { bestDist = d; bestId = pv.id; }
		}
	);

	m.targetPlayerId = bestId;
}

void MonsterCombatSystem::ExecuteAttack(Monster& m, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast, IMonsterClock& clock, const MonsterStats& stats)
{
	const int64_t now = clock.NowMs();
	if (now >= m.nextAttackAtMs) {
		IMonsterEntityLinker::PlayerView pv;
		if (linker.TryGetPlayer(m.targetPlayerId, pv)) {
			int dist = Mdist(m.core.pos, Pos2{ pv.x, pv.y });
			if (dist <= stats.attackRangeTiles) {
				GConsoleLogger->WriteStdOut(Color::WHITE, L"[Combat] Monster:%d attacks player:%d (dmg:%d)\n", m.core.id, m.targetPlayerId, stats.atk);
				linker.ApplyDamageToPlayer(m.targetPlayerId, stats.atk, (int)m.core.id);
				cast.BroadcastMonsterAttack(m.core.id, m.targetPlayerId);
				m.nextAttackAtMs = now + stats.attackCooldownMs;
			}
		}
	}
}
