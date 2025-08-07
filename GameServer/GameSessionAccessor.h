#pragma once
class GameSessionAccessor
{

public:
	explicit GameSessionAccessor(GameSessionContainerRef container)
		: _gameSessionContainer(std::move(container))
	{ }

public:
	void BroadCast(SendBuffer sendBuffer);
	
private:
	GameSessionContainerRef _gameSessionContainer;
};

