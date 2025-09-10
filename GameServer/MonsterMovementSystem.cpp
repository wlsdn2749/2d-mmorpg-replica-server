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


bool MonsterMovementSystem::TryRotate(Monster& m, 
									   Protocol::EDirection targetDir,
									   IMonsterBroadcaster& cast)
{
	if (m.core.dir == targetDir) return false; // 이미 같은 방향
	
	m.core.dir = targetDir;
	m.needsRotation = false;
	cast.BroadcastMonsterMove(m.core.id, m.core.pos.x, m.core.pos.y, targetDir);
	GConsoleLogger->WriteStdOut(Color::GREEN, L"[MonsterMove] Monster:%d rotated to dir:%d\n", 
		m.core.id, (int)targetDir);
	return true;
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
	GConsoleLogger->WriteStdOut(Color::GREEN, L"[MonsterMove] Monster:%d moved to (%d,%d) dir:%d\n", 
		m.core.id, to.x, to.y, (int)dir);
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
			int stepMs = 1000 / monsterStats.moveSpeedTilesPerSec;
			
			// Patrol 시작: 새로운 방향과 스텝 수 설정
			if (m.patrolStepsRemaining <= 0) {
				m.targetDirection = static_cast<Protocol::EDirection>(rng.NextInt(0, 3));
				m.patrolTargetSteps = rng.NextInt(3, 5); // 3-5칸
				m.patrolStepsRemaining = m.patrolTargetSteps;
				m.needsRotation = (m.core.dir != m.targetDirection);
				GConsoleLogger->WriteStdOut(Color::WHITE, L"[Patrol] Monster:%d starting patrol: dir=%d, steps=%d\n", 
					m.core.id, (int)m.targetDirection, m.patrolTargetSteps);
			}
			
			// 회전이 필요하면 회전 먼저
			if (m.needsRotation) {
				if (this->TryRotate(m, m.targetDirection, cast)) {
					m.nextMoveAtMs = clock.NowMs() + stepMs;
				} else {
					m.nextMoveAtMs = clock.NowMs() + 200;
				}
			} else {
				// 이동 시도
				if (this->TryStep(m, m.targetDirection, map, cast)) {
					m.patrolStepsRemaining--;
					m.nextMoveAtMs = clock.NowMs() + stepMs;
				} else {
					// 막혔으면 다른 방향으로 전환
					m.patrolStepsRemaining = 0; // 새 방향 선택 강제
					m.nextMoveAtMs = clock.NowMs() + 200;
				}
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
					Protocol::EDirection targetDir = FaceTo(m.core.pos, Pos2{ pv.x, pv.y });
					if (m.core.dir != targetDir) {
						this->TryRotate(m, targetDir, cast);
					}
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
				m.patrolStepsRemaining = 0; // 새 패트롤 시작
				break;
			}

			IMonsterEntityLinker::PlayerView pv;
			Protocol::EDirection targetDir = Protocol::EDirection::DIR_UP;
			if (linker.TryGetPlayer(m.targetPlayerId, pv)) {
				targetDir = FaceTo(m.core.pos, Pos2{ pv.x, pv.y });
				GConsoleLogger->WriteStdOut(Color::WHITE, L"[Movement] Monster:%d tracking player:%d from (%d,%d) to (%d,%d) \n",
					m.core.id, m.targetPlayerId, m.core.pos.x, m.core.pos.y, pv.x, pv.y);
			}

			int stepMs = 1000 / monsterStats.moveSpeedTilesPerSec;
			
			// 2단계: 회전 먼저, 그 다음 이동
			if (m.core.dir != targetDir) {
				if (this->TryRotate(m, targetDir, cast)) {
					m.nextMoveAtMs = clock.NowMs() + stepMs;
				} else {
					m.nextMoveAtMs = clock.NowMs() + 200;
				}
			} else {
				if (this->TryStep(m, targetDir, map, cast))
					m.nextMoveAtMs = clock.NowMs() + stepMs;
				else
					m.nextMoveAtMs = clock.NowMs() + 200; // 막히면 짧게 대기
			}
			break;
		}
		case MState::Combat:
		{
			// 전투 시 이동 X, 방향만 조정
			if (m.targetPlayerId == -1)
			{
				m.state = MState::Patrol;
				m.wasAttacked = false;
				m.patrolStepsRemaining = 0; // 새 패트롤 시작
				break;
			}

			IMonsterEntityLinker::PlayerView pv;
			if (linker.TryGetPlayer(m.targetPlayerId, pv)) {
				Protocol::EDirection targetDir = FaceTo(m.core.pos, Pos2{ pv.x, pv.y });
				if (m.core.dir != targetDir) {
					this->TryRotate(m, targetDir, cast);
				}
				GConsoleLogger->WriteStdOut(Color::WHITE, L"[Movement] Monster:%d in combat facing player:%d\n",
					m.core.id, m.targetPlayerId);
			}
			break;
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