#include "pch.h"
#include "GameSessionManager.h"
#include <random>

#include "../GameServer/ClientPacketHandler.h"
#include "../GameServer/GameSession.h"
#include "../GameServer/Protocol.pb.h"

GameSessionManager GSessionManager;

void GameSessionManager::Add(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.insert(session);
}

void GameSessionManager::Remove(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.erase(session);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{

	WRITE_LOCK;
	for (GameSessionRef session : _sessions)
	{
		//_sessions를 for-each를 돌다가, sessions를 삭제하거나 수정하거나 하면,
		//_sessions자체가 오염되, UB 발생할 수 있음
		session->Send(sendBuffer); 
	}
}

