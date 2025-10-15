#include "pch.h"
#include "TownRoom.h"
#include "ClientPacketHandler.h"

#include "RoomManager.h"


bool TownRoom::CanEnterTile(int nx, int ny) const
{
	return !_map->IsBlocked(nx, ny);	
}


void TownRoom::InitRoomSystems()
{
	Room::InitRoomSystems();
}

void TownRoom::OnEnter(const PlayerRef& p)
{
	GConsoleLogger->WriteStdOut(Color::WHITE, L"[%d]: [%s] Has Join the [%s].\n", p->playerId, StrToWstr(p->username).c_str(), StrToWstr(RoomName()).c_str());

	Room::OnEnter(p);


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
