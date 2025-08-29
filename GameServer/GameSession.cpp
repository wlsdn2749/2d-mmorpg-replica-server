#include "pch.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"

#include "Player.h"

void GameSession::OnConnected()
{
	_gameSessionContainer->Add(static_pointer_cast<GameSession>(shared_from_this()));
	SetState(State::Connected);
}

void GameSession::OnDisconnected()
{
	if (GetState() == State::InRoom && _currentPlayer)
	{
		CharacterRepository::CharacterStat stat;
		_currentPlayer->GetCharacterStat(stat);
		CharacterRepository::UpdateCharacterStatsAsync(stat); // 연결종료 시, Stats 저장
	}
	_gameSessionContainer->Remove(static_pointer_cast<GameSession>(shared_from_this()));

	// 여기서 룸 Leave 체크

	SetState(State::Disconnected);
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	auto start = std::chrono::high_resolution_clock::now();

	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO : packetId 대역 체크
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{

}