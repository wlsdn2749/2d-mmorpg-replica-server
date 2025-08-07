#include "pch.h"
#include "GameSessionContainer.h"

void GameSessionContainer::Add(const GameSessionRef& session)
{
	WRITE_LOCK;
	_sessions.insert(session);
}

void GameSessionContainer::Remove(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.erase(session);
}
