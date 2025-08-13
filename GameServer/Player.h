#pragma once

class Room; // 전방 선언

class Player
{

/*----------------------------
	Player Fixed Data
----------------------------*/

public:
	uint64				playerId = 0;
	string				username;
	Protocol::EGender	gender{ 0 }; // gender
	Protocol::ERegion	region{ 0 }; // region

/*----------------------------
	Player Move Data
----------------------------*/

public:
	float posX = 0;
	float posY = 0;

	// 현재 바라보는 방향


/*----------------------------
	Player Runtime Links
----------------------------*/
public:
	// Room은 Player보다 오래 살아 있을 수 있으므로 weak_ptr
	std::weak_ptr<Room> room;

	// Session은 Player보다 오래 살아 있을 수 있으므로 weak_ptr
	std::weak_ptr<GameSession> ownerSession;

public:
	void SetRoom(const shared_ptr<Room>& r) {room = r;}
	shared_ptr<Room> GetRoom() const		{return room.lock(); }
};

