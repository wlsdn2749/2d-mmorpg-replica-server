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

	// Get Player

	IMonsterEntityLinker::PlayerView pv;
	if (linker.TryGetPlayer(bestId, pv)) {
		int dist = Mdist(m.core.pos, Pos2{ pv.x, pv.y });

		if (dist <= stats.attackRangeTiles) {  // 1칸 거리
			if (m.state != MState::Combat && m.state != MState::Ready) {
				GConsoleLogger->WriteStdOut(Color::GREEN, L"[Combat] Monster:%d enters Ready state (target player:%d)\n", m.core.id, bestId);
				m.state = MState::Ready;  // 처음 접근시만 Ready
			}
		}
		else{  // 멀리 있고 전투 모드면 추적
			if (m.state == MState::Ready) {
				// Ready 상태에서 멀어지면 Patrol로 복귀
				m.state = MState::Patrol;
				m.targetPlayerId = -1;
			}
			else if (m.state == MState::Combat) {
				// Combat 상태에서 멀어지면 Chase로 전환
				m.state = MState::Chase;
			}
		}
		m.targetPlayerId = bestId;
	}


	// 공격 시도
	if (m.wasAttacked && m.state == MState::Combat)
	{
		const int64_t now = clock.NowMs();
		if (now >= m.nextAttackAtMs) {
			IMonsterEntityLinker::PlayerView pv;
			if (linker.TryGetPlayer(bestId, pv)) {
				int dist = Mdist(m.core.pos, Pos2{ pv.x, pv.y });
				if (dist <= stats.attackRangeTiles) {
					GConsoleLogger->WriteStdOut(Color::WHITE, L"[Combat] Monster:%d attacks player:%d (dmg:%d)\n", m.core.id, bestId, stats.atk);
					linker.ApplyDamageToPlayer(bestId, stats.atk, (int)m.core.id);
					cast.BroadcastMonsterAttack(m.core.id, bestId);
					m.nextAttackAtMs = now + stats.attackCooldownMs;
				}
			}
		}
	}
}