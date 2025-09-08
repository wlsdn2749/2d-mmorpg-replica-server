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
		GConsoleLogger->WriteStdOut(Color::RED, L"[WARNING] ResetSession: 잘못된 상태 - %d\n", (int)GetState());
		detailOut = "InRoom or InGame 상태가 아니므로 로그아웃 불가능";
		return false;
	}

	// 현재 플레이어가 있다면 데이터 저장 및 룸에서 제거
	if (_currentPlayer != nullptr)
	{
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

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[INFO] GameSession Reset 완료 - JWT 로그인 창으로 복귀\n");
	detailOut = "Logout 성공";

	return true;
}

bool GameSession::CharacterSelect(OUT std::string& detailOut)
{
	// 룸 일때만 가능
	auto currentState = GetState();
	if (currentState != State::InRoom)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"[WARNING] ResetSession: 잘못된 상태 - %d\n", (int)GetState());
		detailOut = "InRoom 상태가 아니므로 캐릭터 선택창 이동 불가능";
	}

	// 현재 플레이어가 있다면 데이터 저장 및 룸에서 제거
	if (_currentPlayer != nullptr)
	{
		// 룸에서 플레이어 제거
		RoomRef room = _currentPlayer->GetRoom();
		if (room != nullptr)
		{
			room->DoAsync(&Room::Leave, _currentPlayer); // 여기서 데이터 저장
		}
	}

	SetState(State::InGame); // 캐릭터 선택을 해야하는 상태로 만들기
	_currentPlayer = nullptr; 
	detailOut = "Character Select 성공!";

	return true;
}
