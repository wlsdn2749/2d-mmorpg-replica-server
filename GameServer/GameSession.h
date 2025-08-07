#pragma once

#include "Session.h"
#include "GameSessionContainer.h"

class GameSession : public PacketSession
{
/*----------------------
	GameSession
----------------------*/
public:
	explicit GameSession(const GameSessionContainerRef& container)
		: _gameSessionContainer(container) { }

	virtual ~GameSession()
	{
		cout << "~GameSession" << endl;
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

private:
	GameSessionContainerRef _gameSessionContainer;

/*---------------------------
		Session State
---------------------------*/

public:
	enum class State : uint8
	{
		None, // TCP 연결 전 (가능한가?)...
		Connected, // TCP 연결 된 직후 -> OnConnected() 
		InGame, // jwt 인증 완료 -> ClientPacketHandler:: ... 에서 처리
		InRoom, // 룸에 접속함 -> ClientPacketHandler:: ... 에서 처리
		Disconnected // Room::OnDisConnected
	};

	State	GetState() const {return _state.load(std::memory_order_acquire); }
	void	SetState(State s) {_state.store(s, std::memory_order_release); }

private:
	Atomic<State> _state { State::None };





public:
	PlayerRef _player; // 현재 플레이어 

};

