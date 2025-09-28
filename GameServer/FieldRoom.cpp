#include "pch.h"
#include "FieldRoom.h"
#include "ClientPacketHandler.h"
#include "RoomManager.h"
#include "DropManager.h"
#include "ItemManager.h"
#include "MonsterDataParser.h"
#include "PartyService.h"
#include "SpawnPointDataParser.h"
#include <random>


bool FieldRoom::ProcessMonsterDropInRoom(EntityId monsterId, int killerPlayerId)
{
	// 드랍 아이템, 플레이어 얻기
	auto droppedItems = DropManager::Instance().ProcessAllDrops(monsterId);
	auto player = FindPlayer(killerPlayerId);

	// 실패 여부 먼저 판정
	if(!player || droppedItems.empty()) return false;
	
	int successCount = 0;
	int failCount = 0;
	std::string failedItems ;

	for (const auto& item : droppedItems)
	{
		auto result = player->AddItem(item.itemId, item.count);

		if (result == EAddItemResult::Success)
		{
			++successCount;
			GConsoleLogger->WriteStdOut(Color::GREEN,
				L"Drop Success: PlayerId[%d] ItemId[%d] Count[%d]\n",
				killerPlayerId, item.itemId, item.count); // 성공 로그
		}
		else {
			// 실패 로그
			++failCount;

			std::string itemName = "Unknown Item";
			if (const ItemData* itemData = ItemManager::Instance().GetItemData(item.itemId)) {
				itemName = itemData->name;
			}
			if (!failedItems.empty()) failedItems += ", ";
			failedItems += itemName + " x" + std::to_string(item.count);


			GConsoleLogger->WriteStdOut(Color::RED,
				L"Drop Failed: PlayerId[%d] ItemId[%d] Reason[%s]\n",
				killerPlayerId, item.itemId, result, toString(result));
		}
	}

	// 성공한 아이템이 있으면 인벤토리 업데이트
	if (successCount > 0) {
		SendInventoryUpdateToPlayer(killerPlayerId);
	}

	// 실패한 아이템이 있으면 알림 전송
	if (failCount > 0) {
		std::string message = "인벤토리가 가득 참: " + failedItems;
		SendSystemMessageToPlayer(killerPlayerId, message, Protocol::EMessageType::MESSAGE_WARNING);
	}

	return true;
}

bool FieldRoom::ProcessMonsterMoneyInRoom(EntityId typeId, int srcPlayerId)
{
	const int dropMoney = 500; /*TODO 나중에 sheet 기반으로 변경*/
	auto player = FindPlayer(srcPlayerId);
	player->AddMoney(dropMoney);
	return true;
}

bool FieldRoom::ProcessMonsterExpInRoom(EntityId typeId, int srcPlayerId)
{
	const int dropExp = 100; /*TODO 나중에 sheet 기반으로 변경*/
	auto killerPlayer = FindPlayer(srcPlayerId);

	// 파티 경험치 분배  (PartyService로 위임)
	if (killerPlayer->IsInParty())
	{
		PartyService::Instance().DistributeExp(killerPlayer, dropExp);
	}
	else
	{
		killerPlayer->AddExp(dropExp);
	}
	return true;
}

void FieldRoom::SendInventoryUpdateToPlayer(int killPlayerId)
{
	auto player = FindPlayer(killPlayerId);
	if (!player) return;

	Protocol::S_InventoryUpdate updatePkt;
	auto slots = player->GetInventory().ToProtocolSlots();
	for (const auto& slotInfo : slots) {
		*updatePkt.add_changedslots() = slotInfo;
	}

	if (auto gameSession = player->ownerSession.lock()) {
		gameSession->Send(ClientPacketHandler::MakeSendBuffer(updatePkt));
	}
}

void FieldRoom::SendSystemMessageToPlayer(int playerId, const std::string& message, Protocol::EMessageType type)
{
	auto player = FindPlayer(playerId);
	if (!player) return;

	Protocol::S_SystemMessage pkt;
	pkt.set_message(message);
	pkt.set_type(type);

	if (auto gameSession = player->ownerSession.lock()) {
		gameSession->Send(ClientPacketHandler::MakeSendBuffer(pkt));
	}
}

void FieldRoom::SendMonstersToPlayer(PlayerRef p)
{
	auto pkt = _monsters->BuildMonsterSnapShot(RoomId());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	if (auto s = p->ownerSession.lock())
	{
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
		s->Send(sendBuffer);
	}
}

void FieldRoom::InitRoomSystems()
{
	_lastMonsterTickMs = _clock.NowMs(); // 첫 기준 시각
	InitMonsters();
	InitPCombat();
}


void FieldRoom::OnEnter(const PlayerRef& p)
{
	GConsoleLogger->WriteStdOut(Color::WHITE, L"[%d]: [%s] Has Join the [%s].\n", p->playerId, StrToWstr(p->username).c_str(), StrToWstr(RoomName()).c_str());

	// 플레이어 전송
	{ 
		// 플레이어 스냅샷을 -> 접속한 플레이어에게 전달
		auto pkt = BuildPlayerListSnapshot(p);
		if (auto s = p->ownerSession.lock())
		{
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
			s->Send(sendBuffer);
		}
		
		// 플레이어가 입장함을 -> 다른 플레이어에게 전달
		BroadcastEnter(p);
	}

	// 맵에 있는 몬스터 전송 -> 접속한 플레이어에게 전달 - 나중에 이부분은 합칠 예정
	SendMonstersToPlayer(p);
	
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
void FieldRoom::OnPlayerHpChanged(int playerId)
{
	Room::OnPlayerHpChanged(playerId);
}

// 사망시 알림 (+여기서 리스폰 정책은 추후)
void FieldRoom::OnPlayerDeath(int playerId, int killerMonsterId)
{
	Room::OnPlayerDeath(playerId, killerMonsterId);
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

void FieldRoom::OnRecvAttackReq(const PlayerRef& p, const Protocol::C_PlayerAttackRequest& req)
{
	if (!_pCombat) return;
	if (p->IsDead()) return;

	Room::OnRecvAttackReq(p, req);
	const int64_t nowMs = Time::NowSteadyMs();
	_pCombat->HandleAttack(p, nowMs);
}

/* 어댑터 */

void FieldRoom::MonsterEntityLinkerImpl::ForEachPlayerInRange(int cx, int cy, int rangeTiles, std::function<void(const PlayerView&)> cb) const
{
	for (auto& [pid, player] : _r.Players()) {
		if (!player) continue;
		if (player->IsDead()) continue;
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
	if (p->IsDead()) return false;
	const auto pos = p->GetPos();            // TODO
	out = PlayerView{ pid, pos.x, pos.y, p->Hp() }; // TODO
	return true;
}

void FieldRoom::MonsterEntityLinkerImpl::ApplyDamageToPlayer(int pid, int dmg, int srcMonsterId)
{
	auto p = _r.FindPlayer(pid);
	if (!p) return;
	const bool dead = p->ApplyDamage(dmg, srcMonsterId); // TODO: 실제 시그니처
	_r.OnPlayerHpChanged(pid);
	if (dead) _r.OnPlayerDeath(pid, srcMonsterId);
}

// ------------------- Broadcaster -------------------
void FieldRoom::MonsterBroadcasterImpl::SpawnMonster(const Monster& monster)
{
	Protocol::S_SpawnMonster pkt;
	auto* info = pkt.mutable_monster();
	info->set_monsterid(monster.core.id);
	info->set_monstertypeid(monster.typeId);
	auto* pos = info->mutable_pos();
	pos->set_x(monster.core.pos.x);
	pos->set_y(monster.core.pos.y);

	info->set_direction(monster.core.dir);
	
	GConsoleLogger->WriteStdOut(Color::YELLOW, L"몬스터 스폰 Id: %d, typeId: %d, x: %d, y: %d, dir: %d\n", monster.core.id, monster.typeId, monster.core.pos.x, monster.core.pos.y, (int)monster.core.dir);
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

void FieldRoom::MonsterBroadcasterImpl::BroadcastMonsterAttack(EntityId id, int targetPid, int damage, int hpAfter)
{
	Protocol::S_BroadcastMonsterAttack pkt;
	pkt.set_monsterid(id);
	pkt.set_targetpid(targetPid);
	pkt.set_damage(damage);
	pkt.set_hpafter(hpAfter);
	GConsoleLogger->WriteStdOut(Color::GREEN, L"[Monster] Monster:%d attacks PlayerId:%d (dmg:%d hp:%d->%d)",
		id, targetPid, damage, hpAfter+damage, hpAfter);
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

void FieldRoom::LoadMonsterStatData(MonsterService::Cfg& cfg, std::unique_ptr<vector<pair<int, MonsterStats>>> monsterStatDatas)
{
	// 스탯/스폰 설정 (예시)
	for (auto& [monsterId, monsterStats] : *monsterStatDatas.get())
	{
		cfg.statsByType.emplace(monsterId, monsterStats);
	}
}

void FieldRoom::LoadMonsterSpawnData(MonsterService::Cfg& cfg, std::unique_ptr<vector<SpawnPointCfg>> spawnPointCfgDatas)
{
	for (auto& spawnPointCfg : *spawnPointCfgDatas.get())
	{
		cfg.spawns.push_back(spawnPointCfg);
	}
}

void FieldRoom::InitMonsters()
{
	MonsterService::Cfg cfg;

	// JSON에서 몬스터 데이터 로드
	auto monsterStatDatas = std::make_unique<vector<pair<int, MonsterStats>>>();
	auto spawnPointCfgDatas = std::make_unique<vector<SpawnPointCfg>>();

	try
	{
		// Monster_data.json에서 몬스터 스탯 데이터 로드
		auto monsterDataMap = MonsterDataParser::LoadMonsterData();

		for (const auto& [monsterId, monsterRecord] : monsterDataMap)
		{
			monsterStatDatas->push_back({ monsterId, monsterRecord.stats });
		}

		// SpawnPoint_data.json에서 스폰 데이터 로드
		
		auto spawnPointDataMap = SpawnPointDataParser::LoadSpawnPointData();

		for (const auto& [id, spawnPointRecord] : spawnPointDataMap)
		{
			spawnPointCfgDatas->push_back(spawnPointRecord.cfg);
		}

		//// 1001번 몬스터 (일반 몬스터)
		//if (monsterDataMap.find(1001) != monsterDataMap.end())
		//{

		//	SpawnPointCfg spawnPointCfg = { 1, 15, -4, 5, 1, 8000, 10, 1001 };
		//	spawnPointCfgDatas->push_back(spawnPointCfg);

		//}

		//// 2001번 몬스터 (다른 몬스터 타입)
		//if (monsterDataMap.find(2001) != monsterDataMap.end())
		//{
		//	SpawnPointCfg spawnPointCfg = { 2, 18, -8, 3, 1, 6000, 8, 2001 };
		//	spawnPointCfgDatas->push_back(spawnPointCfg);
		//}

		// 허수아비 몬스터 9999 (하드코딩 유지 - JSON에 없을 경우 대비)
		if (monsterDataMap.find(9999) == monsterDataMap.end())
		{
			// JSON에 허수아비가 없으면 하드코딩으로 생성
			MonsterStats dummyStats = { 9999, 0, 0, 0, 0, 0 };
			monsterStatDatas->push_back({ 9999, dummyStats });
		}

		// 허수아비 스폰 포인트
		SpawnPointCfg dummySpawnPointCfg = { 99, 20, -6, 1, 1, 1000, 0, 9999 };
		spawnPointCfgDatas->push_back(dummySpawnPointCfg);
	}
	catch (const std::exception& e)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"몬스터 데이터 로딩 실패, 기본값 사용: %s\n", StrToWstr(e.what()).c_str());

		// 에러 발생시 기본 하드코딩 값 사용
		MonsterStats defaultStats = { 30, 5, 1, 1, 1200, 6 };
		monsterStatDatas->push_back({ 1001, defaultStats });

		SpawnPointCfg defaultSpawn = { 1, 15, -4, 5, 1, 8000, 10, 1001 };
		spawnPointCfgDatas->push_back(defaultSpawn);
	}

	LoadMonsterStatData(cfg, std::move(monsterStatDatas));
	LoadMonsterSpawnData(cfg, std::move(spawnPointCfgDatas));

	cfg.movementCfg = MonsterMovementSystem::Cfg(800, 1600);

	// ★ 퍼사드 생성자 인자 타입이 새 포트명으로 맞춰져 있어야 함
	_monsters = std::make_unique<MonsterService>(_mapQuery, _linker, _cast, _clock, _rng);
	_monsters->Init(cfg);
}

void FieldRoom::InitPCombat()
{
	PlayerCombatSystem::Cfg cfg;
	cfg.attackCooldownMs = 500; // PRD 1틱~2틱 안에 재공격 금지용 예시
	_pCombat = std::make_unique<PlayerCombatSystem>(cfg, _pLinker, _pCaster);
}

/*--------------------------------
		플레이어 어댑터
--------------------------------*/
void FieldRoom::PlayerMonsterLinkerImpl::ForEachMonsterInRange(int cx, int cy, int rangeTiles, std::function<void(const MonsterView&)>) const
{
	// TODO Implementation
}

bool FieldRoom::PlayerMonsterLinkerImpl::TryGetMonster(EntityId monsterId, MonsterView& outMv) const
{
	if(!_r._monsters->TryGetMonsterView(monsterId, outMv)) return false;
	return true;
}

bool FieldRoom::PlayerMonsterLinkerImpl::TryGetMonsterAt(int fx, int fy, MonsterView& outMonster) const
{
	// 해당 위치에 있는 몬스터를 찾기
	bool found = false;
	_r._monsters->ForEachMonsterView([&](const MonsterView& mv) {
		if (mv.x == fx && mv.y == fy) {
			outMonster = mv;
			found = true;
			return; // 첫 번째 찾은 몬스터만 반환
		}
	});
	return found;
}

bool FieldRoom::PlayerMonsterLinkerImpl::ApplyDamageToMonster(int monsterId, int damage, int srcPlayerId)
{
	MonsterService::MonsterView mv;
	if (!_r._monsters->TryGetMonsterView(monsterId, mv)) return false;

	int hpAfter = 0;
	bool isDie = false;
	_r._monsters->ApplyDamageToMonster(monsterId, damage, srcPlayerId, OUT hpAfter, OUT isDie);

	// 몬스터가 죽었는지 확인 (hpAfter <= 0)
	if (isDie && hpAfter <= 0)
	{
		// TODO 경험치 시스템 - Cfg
		_r.ProcessMonsterExpInRoom(mv.typeId, srcPlayerId);
		// TODO 돈 지급 - Cfg
		_r.ProcessMonsterMoneyInRoom(mv.typeId, srcPlayerId);
		//_r.ProcessMonsterDropInRoom(mv.typeId, srcPlayerId); // 플레이어에게 드랍

		auto player = _r.FindPlayer(srcPlayerId);
		_r.OnPlayerStatChanged(player);
	}
	
	return isDie;

}


void FieldRoom::PlayerCombatBroadcasterImpl::BroadcastPlayerAttack(int attackerId, int targetId, int damage, int hpAfter)
{
	Protocol::S_BroadcastPlayerAttack pkt;
	pkt.set_playerid(attackerId);
	pkt.set_targetid(targetId);
	pkt.set_damage(damage);
	pkt.set_hpafter(hpAfter);
	_r.Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}
