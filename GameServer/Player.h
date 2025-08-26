#pragma once
#include "TypeCore.h"       // EntityId, EntityKind
#include "GeometryCore.h"   // Pos2, Dir
#include "EntityCore.h"     // EntityCore

class Room; // 전방 선언

struct PendingRoomChange {
	bool active = false;
	int transitionId = 0;
	int dstMapId = 0;
	int dstPortalId = 0;
};

class Player
{

/*----------------------------
	Player Fixed Data
----------------------------*/

public:
	EntityId			playerId { 0 }; // DB에서 CharacterId임
	string				username;
	Protocol::EGender	gender{ 0 }; // gender
	Protocol::ERegion	region{ 0 }; // region

/*------------------------------------------
	Player Move Data (Core) / (위치, 방향)
------------------------------------------*/
public:
	EntityCore core{
		/*id   */ 0,
		/*kind */ EntityKind::Player,
		/*pos  */ {0,0},
		/*dir  */ Protocol::EDirection::DIR_DOWN
	};

	inline Pos2 GetPos() const { return { core.pos.x, core.pos.y }; }
	inline void SetPos(int x, int y) { core.pos = { x,y }; }
	inline void SetDir(Protocol::EDirection d) { core.dir = d; }

/*-------------------------------
	HP 등 나중에 수정 할 예정 
-------------------------------*/
public:
	int _hp { 30 };
	inline int Hp() const {return _hp; }
	bool ApplyDamage(int dmg, int srcMonsterId) {
		_hp = std::max(0, _hp - std::max(0, dmg));
		return (_hp == 0); // dead?
	}

/*---------------------------------
	Player Room Transitioning Data
---------------------------------*/
public:
	int NextTransitionId() { return ++_lastTransitionId; }

	void BeginRoomChangeState(int tid, int dstMapId, int dstPortalId) {
		_transferring = true;
		_pending.active = true;
		_pending.transitionId = tid;
		_pending.dstMapId = dstMapId;
		_pending.dstPortalId = dstPortalId;
	}
	void ClearRoomChangeState() {
		_transferring = false;
		_pending = {};
	}

	bool IsTransferring() const { return _transferring; }
	const PendingRoomChange& PendingChange() const { return _pending; }

private:
	int _lastTransitionId = 0;
	bool _transferring = false;
	PendingRoomChange _pending;

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

