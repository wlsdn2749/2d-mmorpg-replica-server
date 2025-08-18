#include "pch.h"
#include "Room.h"
#include "ClientPacketHandler.h"
#include "MapData.h"

// ctor/dtor
Room::Room(Room::Cfg cfg) : _cfg(std::move(cfg)) {}
Room::Room(Cfg cfg, std::shared_ptr<const MapData> map)
    : _cfg(std::move(cfg)), _map(std::move(map))
{
    if (!_map) throw std::invalid_argument("Room requires non-null MapData");
}
Room::~Room() = default;

void Room::StartTicking()
{
	// 첫 틱 예약 (이후 OnTickTimer에서 자기 재등록)
	DoTimer(_cfg.tickMs, &Room::OnTickTimer);
}

PlayerRef Room::FindPlayer(PlayerId pid)
{
	auto it = _players.find(pid);
	return (it == _players.end()) ? nullptr : it->second;
}


Protocol::EDirection Room::DecideFacing(const PlayerRef& p, const Protocol::Vector2Info& clickWorldPos)
{
	int dx = clickWorldPos.x() - p->posX;
	int dy = clickWorldPos.y() - p->posY;

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

/*------------------------
	내부 구현 (룸 스레드)
------------------------*/
void Room::Enter(PlayerRef p, Protocol::EEnterReason enterReason)
{
	// player가 nullptr 일경우
	if(!p) return;

	// 정원 체크?

	// 중복 방지
	if(_players.find(p->playerId) != _players.end())
		return;

	// 방 멤버로 등록
	_players[p->playerId] = p;
	
	p->SetRoom(SharedThis());

    // 만약 맵 이동이라면?
    if (enterReason == Protocol::EEnterReason::ENTER_CHANGE_ROOM)
    {
        OnEnterSetSpawn(p);
    }
	GConsoleLogger->WriteStdOut(Color::GREEN, L"Room에 ENter 입장\n");
	OnEnter(p); // 여기서 모두에게 BroadCasting?
}

void Room::Leave(PlayerRef p)
{
	auto leavePlayerId = p->playerId;
	_players.erase(leavePlayerId);

	p->SetRoom(nullptr); // 약참조 해제
	OnLeave(p); // 여기서 모두에게 BroadCasting?
}

void Room::Broadcast(const SendBufferRef& pkt, PlayerId except)
{
	// TODO: 필요하면 예외 적용
	
	for (auto& [pid, player] : _players)
	{
		if(pid == except) continue;
		if(auto s = player->ownerSession.lock())
			s->Send(pkt);
	}
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

bool Room::CanEnterTile(const PlayerRef& p, int nx, int ny) const
{
    UNREFERENCED_PARAMETER(p);
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

        // 1) 원하는 방향
        auto want = DecideFacing(p, st.pending.clickWorldPos);

        Protocol::EMoveResult result = Protocol::EMoveResult::OK;
        bool anyChange = false;
        int oldX = p->posX, oldY = p->posY;

        // 2) 회전 우선
        if (p->dir != want)
        {
            if (_tick - st.lastActionTick >= _cfg.rotateCooldownTicks)
            {
                p->dir = want;
                st.lastActionTick = _tick;
                anyChange = true;
            }
            else
            {
                result = Protocol::EMoveResult::COOLDOWN;
            }
        }
        else
        {
            // 3) 이동 시도
            if (_tick - st.lastActionTick >= _cfg.moveCooldownTicks)
            {
                int nx = p->posX;
                int ny = p->posY;
                switch (p->dir) {
                case Protocol::EDirection::DIR_UP:    ny -= 1; break;
                case Protocol::EDirection::DIR_DOWN:  ny += 1; break;
                case Protocol::EDirection::DIR_LEFT:  nx -= 1; break;
                case Protocol::EDirection::DIR_RIGHT: nx += 1; break;
                default: break;
                }

                // 예약/충돌 체크
                if (!IsTileReserved(nx, ny) && CanEnterTile(p, nx, ny))
                {
                    ReserveTile(nx, ny);
                    p->posX = nx; p->posY = ny;
                    st.lastActionTick = _tick;
                    anyChange = true;

                    OnPlayerMoved(p, oldX, oldY);
                }
                else
                {
                    result = Protocol::EMoveResult::BLOCKED;
                }
            }
            else
            {
                result = Protocol::EMoveResult::COOLDOWN;
            }
        }

        // 4) 변경 누적 (delta만 브로드캐스트)
        if (anyChange)
        {
            Protocol::PlayerMoveInfo info;
            info.set_playerid(p->playerId);
            info.set_direction(p->dir);
            auto* pos = info.mutable_newpos();
            pos->set_x(p->posX);
            pos->set_y(p->posY);
            changed.emplace_back(std::move(info));
        }

        // ACK: 요청 보낸 본인에게만 전송
        SendMoveAck(p, st.pending.clientSeq, anyChange ? Protocol::EMoveResult::OK : result);
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
    pos->set_x(p->posX);
    pos->set_y(p->posY);
    reply.set_direction(p->dir);

    if (auto s = p->ownerSession.lock())
    {
        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(reply);
        s->Send(sendBuffer);
    }
}