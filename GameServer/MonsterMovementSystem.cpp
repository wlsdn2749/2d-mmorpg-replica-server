#include "pch.h"
#include "MonsterMovementSystem.h"
#include "MonsterSpawnerSystem.h"

#include <algorithm>
#include <cmath>


void MonsterMovementSystem::Tick(MonsterContainer& repo,
								 const MonsterSpawnerSystem& spawner,
								 IMonsterMapQuery& map,
								 IMonsterEntityLinker& linker,	
								 IMonsterBroadcaster& cast,
								 IMonsterClock& clock,
								 IMonsterRng& rng)
{
	const int64_t now = clock.NowMs();
	repo.ForEachMonster([&](Monster& m) {
		if (m.state == MState::Dead) return;
		if (m.nextMoveAtMs > now) return;
		this->TickOne(m, spawner, map, linker, cast, clock, rng);
		});
}


bool MonsterMovementSystem::TryStep(Monster& m, 
									Protocol::EDirection dir, 
									IMonsterMapQuery& map, 
									IMonsterBroadcaster& cast) 
{
	Pos2 d = DirToDelta4(dir);
	Pos2 to{ m.core.pos.x + d.x, m.core.pos.y + d.y };
	if (!map.CanEnter(to.x, to.y)) return false;
	m.core.pos = to;
	m.core.dir = dir;
	cast.BroadcastMonsterMove(m.core.id, to.x, to.y, dir);
	return true;
}


void MonsterMovementSystem::TickOne(Monster& m, 
									const MonsterSpawnerSystem& spawner,
									IMonsterMapQuery& map, 
									IMonsterEntityLinker& linker,
									IMonsterBroadcaster& cast, 
									IMonsterClock& clock, 
									IMonsterRng& rng) 
{
	// 간단 FSM: Idle/Patrol ↔ Chase ↔ Return
	// v1: Chase 판단은 CombatSystem에서 타깃을 세팅한다 가정하고, 여기서는 위치만 담당

	// 리쉬 체크(스폰 반경 2배 이상이면 Return)
	const auto& spawns = spawner.Spawns();
	auto it = std::find_if(spawns.begin(), spawns.end(),
		[&](const SpawnPointCfg& s) { return s.id == m.fromSpawnId; });
	const int leash = (it != spawns.end()) ? it->leashRadiusTiles : 10;
	const int dx = std::abs(m.core.pos.x - m.spawnX);
	const int dy = std::abs(m.core.pos.y - m.spawnY);
	bool outOfLeash = (dx + dy) > leash * 2; // 맨해튼 기준 대충


	if (outOfLeash) m.state = MState::Return;

	auto monsterStats = spawner.GetStats(m.typeId);
	if (monsterStats.moveSpeedTilesPerSec <= 0) // 0보다 작으면 아무런행동 X, MState도 Idle에서 고정
	{
		return;
	}

	switch (m.state)
	{
		case MState::Idle:
		case MState::Patrol:
		{
			// 1~3타일 랜덤 한칸씩 이동 시도
			Protocol::EDirection dir = static_cast<Protocol::EDirection>(rng.NextInt(0, 3));
			if (this->TryStep(m, dir, map, cast))
			{
				// 다음 이동 쿨다운(속도 기반): 1000 / tilesPerSec
				int stepMs = 1000 / monsterStats.moveSpeedTilesPerSec;
				m.nextMoveAtMs = clock.NowMs() + stepMs;
			}
			else
			{
				m.nextMoveAtMs = clock.NowMs() + 200; // 막히면 짧게 대기
			}
			m.state = MState::Patrol;
			break;
		}
		case MState::Ready:
		{
			int stepMs = 1000 / monsterStats.moveSpeedTilesPerSec;
			if (m.targetPlayerId != -1) {
				IMonsterEntityLinker::PlayerView pv;
				if (linker.TryGetPlayer(m.targetPlayerId, pv)) {
					m.core.dir = FaceTo(m.core.pos, Pos2{ pv.x, pv.y });
					GConsoleLogger->WriteStdOut(Color::GREEN, L"[Movement] Monster:%d facing player:%d dir:%d \n", 
						m.core.id, m.targetPlayerId, (int)m.core.dir);
				}
			}
			m.nextMoveAtMs = clock.NowMs() + stepMs;
			break;
		}
		case MState::Chase:
		{
			if (m.targetPlayerId == -1)
			{
				m.state = MState::Patrol;
				m.wasAttacked = false;
				break;
			}

			IMonsterEntityLinker::PlayerView pv;
			Protocol::EDirection dir = Protocol::EDirection::DIR_UP; // 
			if (linker.TryGetPlayer(m.targetPlayerId, pv)) {
				dir = FaceTo(m.core.pos, Pos2{ pv.x, pv.y });
				GConsoleLogger->WriteStdOut(Color::WHITE, L"[Movement] Monster:%d tracking player:%d from (%d,%d) to (%d,%d) \n",
					m.core.id, m.targetPlayerId, m.core.pos.x, m.core.pos.y, pv.x, pv.y);
			}

			int stepMs = 1000 / monsterStats.moveSpeedTilesPerSec;
			if (this->TryStep(m, dir, map, cast))
				m.nextMoveAtMs = clock.NowMs() + stepMs;
			else
				m.nextMoveAtMs = clock.NowMs() + 200; // 막히면 짧게 대기
			break;
		}
		case MState::Combat:
		{
			// 전투 시 이동 X
			if (m.targetPlayerId == -1)
			{
				m.state = MState::Patrol;
				m.wasAttacked = false;
				break;
			}

			IMonsterEntityLinker::PlayerView pv;
			Protocol::EDirection dir = Protocol::EDirection::DIR_UP; // 
			if (linker.TryGetPlayer(m.targetPlayerId, pv)) {
				dir = FaceTo(m.core.pos, Pos2{ pv.x, pv.y });
				GConsoleLogger->WriteStdOut(Color::WHITE, L"[Movement] Monster:%d Entering Combat System player:%d from (%d,%d) to (%d,%d) \n",
					m.core.id, m.targetPlayerId, m.core.pos.x, m.core.pos.y, pv.x, pv.y);
			}
		}
		case MState::Return:
		{
			// TODO 스폰 지점으로 복귀
			break;
		}
		case MState::Dead:
		default:
			break;
	}
}