#pragma once

class GameSession;

using GameSessionRef = shared_ptr<GameSession>;

class GameSessionContainer
{
public:
	void Add(const GameSessionRef& session);
	void Remove(GameSessionRef session);

private:
	USE_LOCK;
	Set<GameSessionRef> _sessions;
};

