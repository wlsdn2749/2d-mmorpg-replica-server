#include "pch.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "RoomManager.h"

void GameSession::OnConnected()
{
	_gameSessionContainer->Add(static_pointer_cast<GameSession>(shared_from_this()));
	SetState(State::Connected);
}

void GameSession::OnDisconnected()
{
	if (GetState() == State::InRoom && _currentPlayer)
	{
		// 게임에서 나갈 때하는 행동 정의
		OnLeaveGame();

		RoomRef room = _currentPlayer->GetRoom();
		room->DoAsync(&Room::Leave, _currentPlayer); // 여기서 데이터 저장
	}

	_gameSessionContainer->Remove(static_pointer_cast<GameSession>(shared_from_this()));

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

bool GameSession::Logout(OUT std::string& detailOut)
{
	// 현재 상태 확인, InRoom 또는 InGame 상태에서만 리셋 가능
	auto currentState = GetState();
	if (currentState != State::InGame && currentState != State::InRoom)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"[WARNING] ResetSession: Wrong State - %d \n", (int)GetState());
		detailOut = "InRoom or InGame Doesn't make sense to logout";
		return false;
	}

	// 현재 플레이어가 있다면 데이터 저장 및 룸에서 제거
	if (_currentPlayer != nullptr)
	{
		// 게임에서 나갈 때하는 행동 정의
		OnLeaveGame();
		// 룸에서 플레이어 제거
		RoomRef room = _currentPlayer->GetRoom();
		if (room != nullptr)
		{
			room->DoAsync(&Room::Leave, _currentPlayer); // 여기서 데이터 저장
		}
	}

	// Jwt 인증 전 상태로 만들기
	SetState(State::Connected);

	_players.clear();
	_currentPlayer = nullptr;
	_account = nullptr;

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[INFO] GameSession Reset Completed - JWT Return to Login Scene\n");
	detailOut = "Logout OK";

	return true;
}

bool GameSession::CharacterSelect(OUT std::string& detailOut)
{
	// 룸 일때만 가능
	auto currentState = GetState();
	if (currentState != State::InRoom)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"[WARNING] ResetSession: Wrong State %d\n", (int)GetState());
		detailOut = "Not InRoom State, Doesn't make sense to move character select";
	}

	// 현재 플레이어가 있다면 데이터 저장 및 룸에서 제거
	if (_currentPlayer != nullptr)
	{
		// 게임에서 나갈때 하는 행동 정의
		OnLeaveGame();
		// 룸에서 플레이어 제거
		RoomRef room = _currentPlayer->GetRoom();
		if (room != nullptr)
		{
			room->DoAsync(&Room::Leave, _currentPlayer); // 여기서 데이터 저장
		}
	}

	SetState(State::InGame); // 캐릭터 선택을 해야하는 상태로 만들기
	_currentPlayer = nullptr; 
	detailOut = "Character Select Success!";

	return true;
}


// _currentPlayer가 해제되기전 가장 먼저 해야함
void GameSession::OnLeaveGame()
{
	// 파티 있으면 파티 해산 
	if(_currentPlayer != nullptr)
		PartyManager::Instance().LeaveParty(_currentPlayer);
}
