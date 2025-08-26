#include "pch.h"
#include "TownRoom.h"
#include "ClientPacketHandler.h"

#include "RoomManager.h"


bool TownRoom::CanEnterTile(int nx, int ny) const
{
	return !_map->IsBlocked(nx, ny);	
}

void TownRoom::StartTick()
{
	;
}

void TownRoom::OnEnter(const PlayerRef& p)
{
	GConsoleLogger->WriteStdOut(Color::WHITE, L"%d: %s가 마을에 입장하셨습니다.\n", p->playerId, p->username);

	// 1. 내 클라에 현재 월드 스냅샷 (기존 유저들) 전송
	auto pkt = BuildPlayerListSnapshot(p);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Broadcast(sendBuffer);
	
	// 2. 기존 유저들에게 나 등장 브로드캐스트
	BroadcastEnter(p);
}

void TownRoom::OnLeave(const PlayerRef& p)
{
	BroadcastLeave(p);
}


void TownRoom::OnRoomTick()
{
	Room::OnRoomTick();
}

void TownRoom::OnPlayerMoved(const PlayerRef& p, int ox, int oy)
{
	// 도착 타일이 만약 맵 이동 타일이라면?
	const int toX = p->core.pos.x;
	const int toY = p->core.pos.y;

	if (const PortalLink* link = _map->GetPortalAt(toX, toY))
	{
		GConsoleLogger->WriteStdOut(Color::GREEN, L"[ChangeRoomBegin!!]: Room 이동시작 \n");
		ChangeRoomBegin(p, *link); // S_ChangeRoomBegin -> Ready -> Commit
		return;
	}
}
