#include "pch.h"
#include "FieldRoom.h"
#include "pch.h"
#include "FieldRoom.h"
#include "ClientPacketHandler.h"
#include "RoomManager.h"
#include <random>

void FieldRoom::StartTick()
{
	_lastMonsterTickMs = _clock.NowMs(); // 첫 기준 시각
}

void FieldRoom::OnEnter(const PlayerRef& p)
{
	GConsoleLogger->WriteStdOut(Color::WHITE, L"%d: %s가 마을에 입장하셨습니다.\n", p->playerId, p->username);

	// 1. 내 클라에 현재 월드 스냅샷 (기존 유저들) 전송
	auto pkt = BuildPlayerListSnapshot(p);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Broadcast(sendBuffer);

	// 2. 기존 유저들에게 나 등장 브로드캐스트
	BroadcastEnter(p);
}

void FieldRoom::OnLeave(const PlayerRef& p)
{
	BroadcastLeave(p);
}


void FieldRoom::OnRoomTick()
{
	// 1) Room 기본 이동/예약 처리
	Room::OnRoomTick();

	// 2) 몬스터 틱 간격 계산
	const int64_t now = _clock.NowMs();
	int64_t dtMs = now - _lastMonsterTickMs;
	if (dtMs < 0) dtMs = 0;          // 방어적
	_lastMonsterTickMs = now;

	// 3) 몬스터 처리 (스폰, 이동, 공격)
	if (_monsters)
		_monsters->Tick(dtMs);

}

// HP 변경시 알림
void FieldRoom::OnPlayerHpChanged(int playerId, int newHp)
{
	GConsoleLogger->WriteStdOut(Color::GREEN, L"playerID: %d 가 newHp : %d가 OnPlayerHpChanged", playerId, newHp);
	//Protocol::S_BroadcastPlayerHp pkt;
	//pkt.set_playerid(playerId);
	//pkt.set_hp(newHp);

	//Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

// 사망시 알림 (+여기서 리스폰 정책은 추후)
void FieldRoom::OnPlayerDeath(int playerId, int killerMonsterId)
{
	//Protocol::S_BroadcastPlayerDeath pkt;
	//pkt.set_playerid(playerId);
	//pkt.set_killermonsterid(killerMonsterId);

	//Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));

	// TODO: 리스폰 규칙(즉시 리스폰/딜레이/마을 복귀 등)은 필요 시 여기서 처리
}

void FieldRoom::OnPlayerMoved(const PlayerRef& p, int ox, int oy)
{
	// 도착 타일이 만약 맵 이동 타일이라면?
	const int toX = p->core.pos.x;
	const int toY = p->core.pos.y;

	if (const PortalLink* link = _map->GetPortalAt(toX, toY))
	{
		ChangeRoomBegin(p, *link); // S_ChangeRoomBegin -> Ready -> Commit
		return;
	}
}

/* 어댑터 */

void FieldRoom::MonsterEntityLinkerImpl::ForEachPlayerInRange(int cx, int cy, int rangeTiles, std::function<void(const PlayerView&)> cb) const
{
	for (auto& [pid, player] : _r.Players()) {
		if (!player) continue;
		const auto pos = player->GetPos();
		const int  hp = player->Hp();      // TODO: "
		int dist = std::abs(pos.x - cx) + std::abs(pos.y - cy);
		if (dist <= rangeTiles) cb(PlayerView{ pid, pos.x, pos.y, hp });
	}
}


bool FieldRoom::MonsterEntityLinkerImpl::TryGetPlayer(int pid, PlayerView& out) const
{
	auto p = _r.FindPlayer(pid);
	if (!p) return false;
	const auto pos = p->GetPos();            // TODO
	out = PlayerView{ pid, pos.x, pos.y, p->Hp() }; // TODO
	return true;
}

void FieldRoom::MonsterEntityLinkerImpl::ApplyDamageToPlayer(int pid, int dmg, int srcMonsterId)
{
	auto p = _r.FindPlayer(pid);
	if (!p) return;
	const bool dead = p->ApplyDamage(dmg, srcMonsterId); // TODO: 실제 시그니처
	_r.OnPlayerHpChanged(pid, p->Hp());
	if (dead) _r.OnPlayerDeath(pid, srcMonsterId);
}

// ------------------- Broadcaster -------------------
void FieldRoom::MonsterBroadcasterImpl::SpawnMonster(const Monster& m)
{
	Protocol::S_SpawnMonster pkt;
	pkt.set_monsterid(m.core.id);
	pkt.set_monstertypeid(m.typeId);
	pkt.set_x(m.core.pos.x);
	pkt.set_y(m.core.pos.y);
	pkt.set_dir(m.core.dir);
	GConsoleLogger->WriteStdOut(Color::YELLOW, L"몬스터 스폰 Id: %d, x: %d, y: %d, dir: %d\n", m.core.id, m.core.pos.x, m.core.pos.y, (int)m.core.dir);
	_r.Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void FieldRoom::MonsterBroadcasterImpl::DespawnMonster(EntityId id, Protocol::EDespawnReason reason)
{
	Protocol::S_DespawnMonster pkt;
	pkt.set_monsterid(id);
	pkt.set_reason(reason);
	_r.Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void FieldRoom::MonsterBroadcasterImpl::BroadcastMonsterMove(EntityId id, int x, int y, Protocol::EDirection dir)
{
	Protocol::S_BroadcastMonsterMove pkt;
	pkt.set_monsterid(id);
	pkt.set_x(x); pkt.set_y(y); pkt.set_dir(dir);
	//GConsoleLogger->WriteStdOut(Color::GREEN, L"몬스터 이동 Id: %d, x: %d, y: %d, dir: %d\n", id, x, y, (int) dir);
	_r.Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void FieldRoom::MonsterBroadcasterImpl::BroadcastMonsterAttack(EntityId id, int targetPid)
{
	Protocol::S_BroadcastMonsterAttack pkt;
	pkt.set_monsterid(id);
	pkt.set_targetpid(targetPid);
	_r.Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void FieldRoom::MonsterBroadcasterImpl::BroadcastMonsterDeath(EntityId id)
{
	Protocol::S_BroadcastMonsterDeath pkt;
	pkt.set_monsterid(id);
	_r.Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

// ------------------- Clock/Rng -------------------
int64_t FieldRoom::MonsterClockImpl::NowMs() const
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

int FieldRoom::MonsterRngImpl::NextInt(int minIncl, int maxIncl)
{
	static thread_local std::mt19937_64 eng{ std::random_device{}() };
	std::uniform_int_distribution<int> dist(minIncl, maxIncl); // [min,max] 포함
	return dist(eng);
}

// ------------------- Monster Service Wiring -------------------

void FieldRoom::InitMonsters()
{
	MonsterService::Cfg cfg;

	// 스탯/스폰 설정 (예시)
	cfg.statsByType.emplace(1001, MonsterStats(
		30, // MaxHp
		5, // atk
		1, // move tile per sec
		1, // atk range tile
		1200, // leash Radius Tiles
		6 // aggroRangeTile
	));

	cfg.spawns.push_back(SpawnPointCfg(
		1,   // id
		20,  // x
		15,  // y
		5,   // maxAlive
		1,   // initialSpawn
		8000,// respawnDelayMs
		10,  // leashRadiusTiles
		1001 // monsterTypeId
	));

	cfg.movementCfg = MonsterMovementSystem::Cfg(800, 1600);

	// ★ 퍼사드 생성자 인자 타입이 새 포트명으로 맞춰져 있어야 함
	_monsters = std::make_unique<MonsterService>(_mapQuery, _linker, _cast, _clock, _rng);
	_monsters->Init(cfg);
}
