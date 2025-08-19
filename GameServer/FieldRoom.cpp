#include "pch.h"
#include "FieldRoom.h"
#include "pch.h"
#include "FieldRoom.h"
#include "ClientPacketHandler.h"

#include "RoomManager.h"


void FieldRoom::StartTick()
{
	;
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
	Room::OnRoomTick();
}

void FieldRoom::OnPlayerMoved(const PlayerRef& p, int ox, int oy)
{
	// 도착 타일이 만약 맵 이동 타일이라면?
	const int toX = p->posX;
	const int toY = p->posY;

	if (const PortalLink* link = _map->GetPortalAt(toX, toY))
	{
		ChangeRoomBegin(p, *link); // S_ChangeRoomBegin -> Ready -> Commit
		return;
	}
}
