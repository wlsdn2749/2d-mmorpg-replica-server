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
	// 몬스터 스탯 가져오기
	auto monsterStats = spawner.GetStats(m.typeId);
	if (monsterStats.moveSpeedTilesPerSec <= 0) {
		// 이동 불가능한 몬스터는 아무 행동도 하지 않음
		return;
	}

	// 1단계: 상태 전환 체크
	CheckStateTransitions(m, spawner, linker, clock, monsterStats);

	// 2단계: 현재 상태에 따른 행동 실행
	switch (m.state) {
		case MState::Idle:
			// Idle은 자동으로 Patrol로 전환
			m.state = MState::Patrol;
			[[fallthrough]];
		case MState::Patrol:
			ExecutePatrolState(m, map, cast, clock, rng, monsterStats);
			break;
		case MState::Ready:
			ExecuteReadyState(m, linker, cast, clock, monsterStats);
			break;
		case MState::Combat:
			ExecuteCombatState(m, linker, cast, clock, monsterStats);
			break;
		case MState::Chase:
			ExecuteChaseState(m, linker, map, cast, clock, monsterStats);
			break;
		case MState::Return:
			ExecuteReturnState(m, map, cast, clock, monsterStats);
			break;
		case MState::Dead:
		default:
			// 죽었거나 알 수 없는 상태에서는 아무것도 하지 않음
			break;
	}
}

// ===== 새로운 FSM 구조 =====

void MonsterMovementSystem::CheckStateTransitions(Monster& m, 
												  const MonsterSpawnerSystem& spawner,
												  IMonsterEntityLinker& linker, 
												  IMonsterClock& clock,
												  const MonsterStats& stats)
{
	// 1. 리쉬 체크 (최우선)
	if (IsOutOfLeash(m, spawner)) {
		m.state = MState::Return;
		return;
	}

	// 2. 죽음 체크
	if (m.curHp <= 0) {
		m.state = MState::Dead;
		return;
	}

	// 3. 타겟 유효성 체크
	int distToTarget = GetDistanceToTarget(m, linker);
	bool hasValidTarget = (m.targetPlayerId != -1 && distToTarget != -1);
	bool inAttackRange = hasValidTarget && IsPlayerInAttackRange(m, linker, stats.attackRangeTiles);

	// 4. 상태별 전환 규칙
	switch (m.state) {
		case MState::Idle:
		case MState::Patrol:
		{
			if (hasValidTarget && inAttackRange) {
				m.state = MState::Ready;
				GConsoleLogger->WriteStdOut(Color::GREEN, L"[StateTransition] Monster:%d Patrol->Ready (target:%d)\n", 
					m.core.id, m.targetPlayerId);
			}
			break;
		}
		case MState::Ready:
		{
			if (m.wasAttacked) {
				m.state = MState::Combat;
				GConsoleLogger->WriteStdOut(Color::GREEN, L"[StateTransition] Monster:%d Ready->Combat (attacked)\n", 
					m.core.id);
			}
			else if (!hasValidTarget || !inAttackRange) {
				m.state = MState::Patrol;
				m.targetPlayerId = -1;
				m.patrolStepsRemaining = 0; // 새 패트롤 시작
				GConsoleLogger->WriteStdOut(Color::GREEN, L"[StateTransition] Monster:%d Ready->Patrol (lost target)\n", 
					m.core.id);
			}
			break;
		}
		case MState::Combat:
		{
			if (!hasValidTarget) {
				m.state = MState::Patrol;
				m.targetPlayerId = -1;
				m.wasAttacked = false;
				m.patrolStepsRemaining = 0;
				GConsoleLogger->WriteStdOut(Color::GREEN, L"[StateTransition] Monster:%d Combat->Patrol (no target)\n", 
					m.core.id);
			}
			else if (!inAttackRange) {
				m.state = MState::Chase;
				GConsoleLogger->WriteStdOut(Color::GREEN, L"[StateTransition] Monster:%d Combat->Chase (target far)\n", 
					m.core.id);
			}
			break;
		}
		case MState::Chase:
		{
			if (!hasValidTarget) {
				m.state = MState::Patrol;
				m.targetPlayerId = -1;
				m.wasAttacked = false;
				m.patrolStepsRemaining = 0;
				GConsoleLogger->WriteStdOut(Color::GREEN, L"[StateTransition] Monster:%d Chase->Patrol (no target)\n", 
					m.core.id);
			}
			else if (inAttackRange) {
				m.state = MState::Combat;
				GConsoleLogger->WriteStdOut(Color::GREEN, L"[StateTransition] Monster:%d Chase->Combat (in range)\n", 
					m.core.id);
			}
			break;
		}
		case MState::Return:
		{
			// TODO: 스폰 지점 도달 시 Patrol로 복귀
			break;
		}
		case MState::Dead:
		default:
			break;
	}
}

// 유틸리티 함수들
int MonsterMovementSystem::GetDistanceToTarget(Monster& m, IMonsterEntityLinker& linker)
{
	if (m.targetPlayerId == -1) return -1;
	
	IMonsterEntityLinker::PlayerView pv;
	if (!linker.TryGetPlayer(m.targetPlayerId, pv)) return -1;
	
	return std::abs(m.core.pos.x - pv.x) + std::abs(m.core.pos.y - pv.y);
}

bool MonsterMovementSystem::IsPlayerInAttackRange(Monster& m, IMonsterEntityLinker& linker, int attackRange)
{
	int dist = GetDistanceToTarget(m, linker);
	return (dist != -1 && dist <= attackRange);
}

bool MonsterMovementSystem::IsOutOfLeash(Monster& m, const MonsterSpawnerSystem& spawner)
{
	const auto& spawns = spawner.Spawns();
	auto it = std::find_if(spawns.begin(), spawns.end(),
		[&](const SpawnPointCfg& s) { return s.id == m.fromSpawnId; });
	
	if (it == spawns.end()) return false;
	
	const int leash = it->leashRadiusTiles;
	const int dx = std::abs(m.core.pos.x - m.spawnX);
	const int dy = std::abs(m.core.pos.y - m.spawnY);
	return (dx + dy) > leash * 2; // 맨해튼 기준
}

// ===== 상태별 실행 로직 =====

void MonsterMovementSystem::ExecutePatrolState(Monster& m, IMonsterMapQuery& map, IMonsterBroadcaster& cast, 
											   IMonsterClock& clock, IMonsterRng& rng, const MonsterStats& stats)
{
	int stepMs = 1000 / stats.moveSpeedTilesPerSec;
	
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
}

void MonsterMovementSystem::ExecuteReadyState(Monster& m, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast,
											 IMonsterClock& clock, const MonsterStats& stats)
{
	int stepMs = 1000 / stats.moveSpeedTilesPerSec;
	
	// Ready 상태에서는 이동하지 않고 플레이어 방향으로만 회전
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
}

void MonsterMovementSystem::ExecuteCombatState(Monster& m, IMonsterEntityLinker& linker, IMonsterBroadcaster& cast,
											  IMonsterClock& clock, const MonsterStats& stats)
{
	// Combat 상태에서는 이동하지 않고 플레이어 방향으로만 회전
	if (m.targetPlayerId != -1) {
		IMonsterEntityLinker::PlayerView pv;
		if (linker.TryGetPlayer(m.targetPlayerId, pv)) {
			Protocol::EDirection targetDir = FaceTo(m.core.pos, Pos2{ pv.x, pv.y });
			if (m.core.dir != targetDir) {
				this->TryRotate(m, targetDir, cast);
			}
			GConsoleLogger->WriteStdOut(Color::WHITE, L"[Movement] Monster:%d in combat facing player:%d\n",
				m.core.id, m.targetPlayerId);
		}
	}
}

void MonsterMovementSystem::ExecuteChaseState(Monster& m, IMonsterEntityLinker& linker, IMonsterMapQuery& map,
											 IMonsterBroadcaster& cast, IMonsterClock& clock, const MonsterStats& stats)
{
	if (m.targetPlayerId == -1) return;

	IMonsterEntityLinker::PlayerView pv;
	Protocol::EDirection targetDir = Protocol::EDirection::DIR_UP;
	if (linker.TryGetPlayer(m.targetPlayerId, pv)) {
		targetDir = FaceTo(m.core.pos, Pos2{ pv.x, pv.y });
		GConsoleLogger->WriteStdOut(Color::WHITE, L"[Movement] Monster:%d tracking player:%d from (%d,%d) to (%d,%d)\n",
			m.core.id, m.targetPlayerId, m.core.pos.x, m.core.pos.y, pv.x, pv.y);
	}

	int stepMs = 1000 / stats.moveSpeedTilesPerSec;
	
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
}

void MonsterMovementSystem::ExecuteReturnState(Monster& m, IMonsterMapQuery& map, IMonsterBroadcaster& cast,
											  IMonsterClock& clock, const MonsterStats& stats)
{
	// TODO: 스폰 지점으로 복귀 로직 구현
	// 현재는 단순히 Patrol로 전환
	m.state = MState::Patrol;
	m.targetPlayerId = -1;
	m.wasAttacked = false;
	m.patrolStepsRemaining = 0;
}