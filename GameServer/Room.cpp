#include "pch.h"
#include "Room.h"
#include "ClientPacketHandler.h"
#include "MapData.h"
#include "RoomManager.h"

// ctor/dtor
Room::Room(Room::Cfg cfg) : _cfg(std::move(cfg)) {}
Room::Room(Cfg cfg, std::shared_ptr<MapData> map)
    : _cfg(std::move(cfg)), _map(std::move(map))
{
    if (!_map) throw std::invalid_argument("Room requires non-null MapData");
}
Room::~Room() = default;

void Room::Init()
{
    StartTicking();
    StartPeriodicSave();
}
void Room::StartTicking()
{
	// 첫 틱 예약 (이후 OnTickTimer에서 자기 재등록)
	DoTimer(_cfg.tickMs, &Room::OnTickTimer);
}

void Room::StartPeriodicSave()
{
    DoTimer(_cfg.periodicSaveTicksMs, &Room::SaveAllActivePlayers);
}

void Room::SaveAllActivePlayers()
{
    GConsoleLogger->WriteStdOut(Color::YELLOW, L"플레이어 정보 업데이트 시작\n");
    // Active 구현을 해야하지만 일단은 모든 플레이어 대상으로
    for (auto& [pid, p] : _players)
    {
        CharacterRepository::CharacterStat stat;  // 스택에 한 번만 생성
        p->GetCharacterStat(stat);  // 복사 없이 직접 할당
        CharacterRepository::UpdateCharacterStatsAsync(stat); // const& 전달
    }

    DoTimer(_cfg.periodicSaveTicksMs, &Room::SaveAllActivePlayers);
}

PlayerRef Room::FindPlayer(PlayerId pid)
{
	auto it = _players.find(pid);
	return (it == _players.end()) ? nullptr : it->second;
}

Protocol::EDirection Room::DecideFacing(const PlayerRef& p, const Protocol::Vector2Info& clickWorldPos)
{
    int dx = clickWorldPos.x() - p->core.pos.x;
    int dy = clickWorldPos.y() - p->core.pos.y;

    // 좌표계 정의: y가 위로 감소(타일맵 전통)라면 UP은 dy<0, DOWN은 dy>0
    // 같은경우 좌우 우선
    if (std::abs(dx) >= std::abs(dy))
    {
        return (dx >= 0) ? Protocol::EDirection::DIR_RIGHT : Protocol::EDirection::DIR_LEFT;
    }
    else
    {
        return (dy >= 0) ? Protocol::EDirection::DIR_DOWN : Protocol::EDirection::DIR_UP;
    }
}
void Room::RemovePlayerInternal(int playerId, std::string_view reason)
{
    UNREFERENCED_PARAMETER(reason);

    auto it = _players.find(playerId);
    if (it == _players.end())
        return;

    PlayerRef p = it->second;
    // 타일 예약 해제 로직 실행
    // TODO UnreserveTile(p->posX, p->posY)
    if(p) p->SetRoom(nullptr);
    
    _players.erase(it);

    GConsoleLogger->WriteStdOut(Color::GREEN, L"[Room::RemovePlayerInternal]: 방에서 퇴장 \n");
}

void Room::AddPlayerInternal(PlayerRef p, SpawnPoint spawn, Protocol::EDirection dir)
{
    if(!p) return;

    // 중복 방지
    if (_players.find(p->playerId) != _players.end())
        return;

    p->core.pos.x = spawn.x;
    p->core.pos.y = spawn.y;
    p->core.dir = dir;

    // 룸 소유권
    p->SetRoom(SharedThis());

    // 타일 예약?
    // TODO ReserveTile(spawn.x, spawn.y)

    // 방 멤버로 등록
    _players[p->playerId] = p;

    GConsoleLogger->WriteStdOut(Color::GREEN, L"[Room::AddPlayerInternal]: Player 스폰됨 (%d,%d,%d) \n", spawn.x, spawn.y, dir);
}

Protocol::S_PlayerList Room::BuildPlayerListSnapshot(const PlayerRef& forPlayer, bool includeSelf) const
{
    Protocol::S_PlayerList pkt;
    pkt.set_myplayerid(forPlayer ? forPlayer->playerId : 0);

    auto* out = pkt.mutable_players();
    out->Reserve(static_cast<int>(_players.size()));

    for (const auto& [id, other] : _players)
    {
        if (!other) continue;
        if (!includeSelf && forPlayer && id == forPlayer->playerId) continue;

        Protocol::PlayerInfo* info = out->Add();
        info->set_id(other->playerId);
        info->set_username(other->username);

        auto* pos = info->mutable_pos();
        pos->set_x(other->core.pos.x);
        pos->set_y(other->core.pos.x);

        info->set_direction(other->core.dir);
    }
    return pkt;
}

/*------------------------
	내부 구현 (룸 스레드)
------------------------*/
void Room::Enter(PlayerRef p)
{
    SpawnPoint spawn{ p->core.pos.x, p->core.pos.y };
    AddPlayerInternal(p, spawn, p->core.dir);  // 내부 등록 (SetRoom 포함)

	GConsoleLogger->WriteStdOut(Color::GREEN, L"Room에 ENter 입장\n");
	OnEnter(p); // 여기서 모두에게 BroadCasting?
}

void Room::Leave(PlayerRef p)
{
    RemovePlayerInternal(p->playerId, "Leave");
    GConsoleLogger->WriteStdOut(Color::GREEN, L"Room에서 Leave 퇴장\n");
	OnLeave(p); // 여기서 모두에게 BroadCasting?
}

void Room::Broadcast(const SendBufferRef& sendBuffer, PlayerId except)
{
	// TODO: 필요하면 예외 적용
	
	for (auto& [pid, player] : _players)
	{
		if(pid == except) continue;
		if(auto s = player->ownerSession.lock())
			s->Send(sendBuffer);
	}
}

void Room::BroadcastEnter(const PlayerRef& newcomer)
{
    Protocol::S_BroadcastPlayerEnter pkt;
    auto* newcomerInfo = pkt.mutable_player();
    newcomerInfo->set_id(newcomer->playerId);
    newcomerInfo->set_username(newcomer->username);

    auto* pos = newcomerInfo->mutable_pos();
    pos->set_x(newcomer->core.pos.x);
    pos->set_y(newcomer->core.pos.y);

    newcomerInfo->set_direction(newcomer->core.dir);

    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    Broadcast(sendBuffer, newcomer->playerId);
    GConsoleLogger->WriteStdOut(Color::GREEN, L"[Room::BroadCastEnter]: 다른 사람에게 BroadCast 중계 \n");

}

void Room::BroadcastLeave(const PlayerRef& leaver)
{
    Protocol::S_BroadcastPlayerLeave pkt;
    pkt.set_playerid(leaver->playerId);
    pkt.set_reason(Protocol::ELeaveReason::LEAVE_UNKNOWN); // 일단 Unknown 고정


    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    Broadcast(sendBuffer, leaver->playerId);

    GConsoleLogger->WriteStdOut(Color::GREEN, L"[Boom::BroadcastLeave]: 플레이어가 퇴장하는것을 중계 \n");
}

/*------------------------
		Tick
------------------------*/
void Room::OnTickTimer()
{
	++_tick;

	OnRoomTick();

	DoTimer(_cfg.tickMs, &Room::OnTickTimer);
}

// Room.cpp
void Room::OnPlayerHpChanged(int playerId, int newHp)
{
    UNREFERENCED_PARAMETER(playerId);
    UNREFERENCED_PARAMETER(newHp);
    // 기본: 아무것도 안 함. 파생(Room)에서 필요 시 오버라이드
}

void Room::OnPlayerDeath(int playerId, int killerMonsterId)
{
    UNREFERENCED_PARAMETER(playerId);
    UNREFERENCED_PARAMETER(killerMonsterId);
    // 기본: 아무것도 안 함. 파생(Room)에서 필요 시 오버라이드
}

bool Room::CanEnterTile(int nx, int ny) const
{
    UNREFERENCED_PARAMETER(nx);
    UNREFERENCED_PARAMETER(ny);
    // 베이스는 항상 true. TownRoom 등에서 충돌/벽/경계 체크 구현
    return true;
}

constexpr int OFFSET = 10000; // 좌표 절대값보다 큼 
static inline int64_t Key(int x, int y) 
{
    return ((int64_t)(x + OFFSET) << 32) | (uint32_t)(y + OFFSET); 
}

void Room::ReserveTile(int nx, int ny)
{
    _reserved.insert(Key(nx, ny));
}

bool Room::IsTileReserved(int nx, int ny) const
{
    return _reserved.find(Key(nx, ny)) != _reserved.end();
}

void Room::OnPlayerMoved(const PlayerRef& p, int ox, int oy)
{
    UNREFERENCED_PARAMETER(p);
    UNREFERENCED_PARAMETER(ox);
    UNREFERENCED_PARAMETER(oy);
    // 파생 룸에서 포탈/트리거 처리
}

void Room::StartTick(){}

void Room::OnRoomTick()
{
    ProcessMovesTick();

    _reserved.clear(); // 예약 버퍼 비움
}

void Room::ProcessMovesTick()
{
    std::vector<Protocol::PlayerMoveInfo> changed;

    for (auto& [pid, p] : _players)
    {
        auto it = _pstates.find(pid);
        if (it == _pstates.end()) continue;
        auto& st = it->second;

        if (!st.pending.has) continue;          // 이번 틱 처리할 요청 없음
        st.pending.has = false;                  // 요청 소비

        // 1) 원하는 방향 TODO :: Direction
        auto want = DecideFacing(p, st.pending.clickWorldPos);

        Protocol::EMoveResult result = Protocol::EMoveResult::MOVE_UNKNOWN;
        bool anyChange = false;
        int oldX = p->core.pos.x, oldY = p->core.pos.y;

        // 2) 회전 우선
        if (p->core.dir != want)
        {
            if (_tick - st.lastActionTick >= _cfg.rotateCooldownTicks)
            {
                p->core.dir = want;
                st.lastActionTick = _tick;
                result = Protocol::EMoveResult::MOVE_DIR;
                anyChange = true;
            }
            else
            {
                result = Protocol::EMoveResult::MOVE_COOLDOWN;
            }
        }
        else
        {
            // 3) 이동 시도
            if (_tick - st.lastActionTick >= _cfg.moveCooldownTicks)
            {
                int nx = p->core.pos.x;
                int ny = p->core.pos.y;
                switch (p->core.dir) {
                case Protocol::EDirection::DIR_UP:    ny -= 1; break;
                case Protocol::EDirection::DIR_DOWN:  ny += 1; break;
                case Protocol::EDirection::DIR_LEFT:  nx -= 1; break;
                case Protocol::EDirection::DIR_RIGHT: nx += 1; break;
                default: break;
                }

                // 예약/충돌 체크
                if (!IsTileReserved(nx, ny) && CanEnterTile(nx, ny))
                {
                    ReserveTile(nx, ny);
                    p->core.pos.x = nx; 
                    p->core.pos.y = ny;
                    st.lastActionTick = _tick;
                    anyChange = true;
                    result = Protocol::EMoveResult::MOVE_OK;
                    OnPlayerMoved(p, oldX, oldY);
                }
                else
                {
                    result = Protocol::EMoveResult::MOVE_BLOCKED;
                }
            }
            else
            {
                result = Protocol::EMoveResult::MOVE_COOLDOWN;
            }
        }

        // 4) 변경 누적 (delta만 브로드캐스트)
        if (anyChange)
        {
            Protocol::PlayerMoveInfo info;
            info.set_playerid(p->playerId);
            info.set_direction(p->core.dir);
            auto* pos = info.mutable_newpos();
            pos->set_x(p->core.pos.x);
            pos->set_y(p->core.pos.y);
            info.set_result(result);
            changed.emplace_back(std::move(info));
        }

        // ACK: 요청 보낸 본인에게만 전송
        SendMoveAck(p, st.pending.clientSeq, result);
    }

    if (!changed.empty())
    {
        Protocol::S_BroadcastPlayerMove pkt;
        pkt.set_tick(_tick);
        for (auto& m : changed) {
            *pkt.add_playermoves() = std::move(m);
        }
        Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
    }
}

/*--------------------------------------------------------
	입력 수신 (C -> S) - Handler를 통해 Async로 중계됨
--------------------------------------------------------*/
void Room::OnRecvMoveReq(PlayerRef p, const Protocol::C_PlayerMoveRequest& req)
{
	if (!p) return;
	auto& st = _pstates[p->playerId];
	st.pending.has = true;
	st.pending.clickWorldPos = req.clickworldpos();
	st.pending.clientSeq = 0;
	st.pending.recvTick = _tick;
}

void Room::ChangeRoomBegin(const PlayerRef& p, const PortalLink& link)
{
    // 플레이어에 전환 상태 저장

    if (!p) return;

    if (p->IsTransferring()) return; // 이미 전환중이면 중복 방지

    const int tid = p->NextTransitionId();
    p->BeginRoomChangeState(tid, link.dstMapId, link.dstPortalId); // 플레이어 상태 변화

    // 입력 잠금 & 로딩 시작 신호 -> Client
    Protocol::S_ChangeRoomBegin beginPkt;
    beginPkt.set_transitionid(tid);
    beginPkt.set_mapid(link.dstMapId);

    if (auto s = p->ownerSession.lock())
    {
        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(beginPkt);
        s->Send(sendBuffer);
    }
    GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_ChangeRoomBegin...]: Client는 Server에게 Ready 패킷을 보내야함 \n");
}

void Room::ChangeRoomReady(const PlayerRef& p, const Protocol::C_ChangeRoomReady& pkt)
{
    // 유효성 확인
    if (!p) return;
    const auto& pend = p->PendingChange();

    if (!pend.active || pend.transitionId != pkt.transitionid())
        return;

    // 목적지 룸 찾기
    RoomRef src = static_pointer_cast<Room>(shared_from_this());
    RoomRef dst = RoomManager::Instance().Find(pend.dstMapId);

    if (!dst) {
        p->ClearRoomChangeState();
        return;
    }

    GConsoleLogger->WriteStdOut(Color::YELLOW, L"[ChangeRoomReady]: src, dst에 룸 이동 요청 \n");
    // src 잡큐에서 실행
    src->DoAsync([src, dst, p, pend] {
        // 중복/레이스 방지: 여전히 src에 있는지 확인
        if (p->GetRoom().get() != src.get())
            return;

        // 플레이어가 방에서 나가도록
        src->RemovePlayerInternal(p->playerId, "MapChange");
        src->BroadcastLeave(p);

        // dst 잡큐에서 실행
        dst->DoAsync([src, dst, p, pend] {
            // 목적지 스폰 계산 (DB NO, 인메모리)
            auto sp = dst->ResolveSpawn(pend.dstPortalId);
            if (!sp) {
                // 실패 정책: 간r단히 상태만 해제. (원하면 src로 롤백도 가능)
                p->ClearRoomChangeState();
                return;
            }

            // 인메모리 좌표 반영 후 등록
            p->core.pos.x = sp->x;
            p->core.pos.y = sp->y;
            dst->AddPlayerInternal(p, { sp->x, sp->y }, p->core.dir);

            // Commit + 스냅샷(나 제외 권장)
            Protocol::S_ChangeRoomCommit commit;
            commit.set_transitionid(pend.transitionId);
            *commit.mutable_snapshots() = dst->BuildPlayerListSnapshot(p, /*includeSelf=*/false);
            
            // DB에 저장
            CharacterRepository::CharacterStat stat;  // 스택에 한 번만 생성
            p->GetCharacterStat(stat);  // 복사 없이 직접 할당
            CharacterRepository::UpdateCharacterStatsAsync(stat); // const& 전달

            if (auto s = p->ownerSession.lock())
            {
                s->Send(ClientPacketHandler::MakeSendBuffer(commit));
                GConsoleLogger->WriteStdOut(Color::GREEN, L"[Room::S_ChangeRoomCommit]: 커밋 송신 (룸 이동완료) \n");
            }

            dst->BroadcastEnter(p);
            p->ClearRoomChangeState();
            });
        });
}

void Room::SendMoveAck(PlayerRef p, int clientSeq, Protocol::EMoveResult result)
{
    if (!p) return;
    // 필요 없다면 제거 가능
    Protocol::S_PlayerMoveReply reply;
    reply.set_playerid(p->playerId);
    reply.set_clientseq(clientSeq);
    reply.set_result(result);
    reply.set_tick(_tick);
    auto* pos = reply.mutable_newpos();
    pos->set_x(p->core.pos.x);
    pos->set_y(p->core.pos.y);
    reply.set_direction(p->core.dir);

    if (auto s = p->ownerSession.lock())
    {
        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(reply);
        s->Send(sendBuffer);
    }
}

std::optional<SpawnPoint> Room::ResolveSpawn(int portalId) const
{
    if (_map)
    {
        /*auto it = _map->spawnPoints.find(portalId);
        if (it != _map->spawnPoints.end())
            return it->second;*/

        auto sp = _map->GetSpawnPoint(portalId);
        if(sp != nullptr)
            return *sp;
            
        // 워커블 찾기 필요하면
    }

    return std::nullopt;
}


