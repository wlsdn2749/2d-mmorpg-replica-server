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

	inline int PosX() const {return core.pos.x;}
	inline int PosY() const {return core.pos.y;}
	inline Protocol::EDirection Dir() const {return core.dir; }
	inline Pos2 GetPos() const { return { core.pos.x, core.pos.y }; }
	inline void SetPos(int x, int y) { core.pos = { x,y }; }
	inline void SetDir(Protocol::EDirection d) { core.dir = d; }

/*-------------------------------
	HP 등 전투 수치 + 레벨
-------------------------------*/
public:
	inline int Hp() const { return _hp; }
	inline int Atk() const { return _atk; }
	inline int Def() const {return _def; }
	inline int Level() const {return _level; }

	bool ApplyDamage(int dmg, int srcMonsterId) {
		_hp = std::max(0, _hp - std::max(0, dmg));
		return (_hp == 0); // dead?
	}

public: // TODO 나중에 private로 수정 필
	int _hp { 30 };
	int _maxHp { 30 };
	int _atk { 10 };
	int _def { 5 };
	int _level { 1 };
	int _exp { 0 };


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

