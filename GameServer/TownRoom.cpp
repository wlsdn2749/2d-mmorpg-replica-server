#include "pch.h"
#include "TownRoom.h"

#include "ClientPacketHandler.h"

void TownRoom::StartTick()
{
	;
}

void TownRoom::OnEnter(const PlayerRef& p)
{
	GConsoleLogger->WriteStdOut(Color::WHITE, L"%d: %s가 마을에 입장하셨습니다.\n", p->playerId, p->username);

	// 1. 내 클라에 현재 월드 스냅샷 (기존 유저들) 전송
	SendWorldSnapShotTo(p);
	
	// 2. 기존 유저들에게 나 등장 브로드캐스트
	BroadcastEnter(p);
}

void TownRoom::OnLeave(const PlayerRef& p)
{
	BroadcastLeave(p);
}

void TownRoom::OnEnterSetSpawn(const PlayerRef& p)
{
	p->posX = SpawnX();
	p->posY = SpawnY();
}

void TownRoom::OnRoomTick()
{
	Room::OnRoomTick();
}

void TownRoom::SendWorldSnapShotTo(const PlayerRef& p)
{
	Protocol::S_PlayerList pkt;
	pkt.set_myplayerid(p->playerId); // 내가 누구인지 클라에게 전송
	auto* out = pkt.mutable_players();
	out->Reserve(static_cast<int>(_players.size()));

	for (auto& [otherPid, other] : _players)
	{
		if (!other) continue;

		Protocol::PlayerInfo* info = out->Add();
		info->set_id(other->playerId);
		info->set_username(other->username);

		auto* pos = info->mutable_pos();
		pos->set_x(other->posX);
		pos->set_y(other->posY);

		info->set_direction(other->dir);
	}

	if (auto s = p->ownerSession.lock())
	{
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
		s->Send(sendBuffer);
	}
}

void TownRoom::BroadcastEnter(const PlayerRef& newcomer)
{
	Protocol::S_BroadcastPlayerEnter pkt;
	auto* newcomerInfo = pkt.mutable_player();
	newcomerInfo->set_id(newcomer->playerId);
	newcomerInfo->set_username(newcomer->username);
	
	auto* pos = newcomerInfo->mutable_pos();
	pos->set_x(newcomer->posX);
	pos->set_y(newcomer->posY);

	newcomerInfo->set_direction(newcomer->dir);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Broadcast(sendBuffer, newcomer->playerId);

}

void TownRoom::BroadcastLeave(const PlayerRef& leaver)
{
	Protocol::S_BroadcastPlayerLeave pkt;
	pkt.set_playerid(leaver->playerId);
	pkt.set_reason(Protocol::ELeaveReason::LEAVE_UNKNOWN); // 일단 Unknown 고정
	
	std::cout << "" << std::endl;
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Broadcast(sendBuffer, leaver->playerId);
}
