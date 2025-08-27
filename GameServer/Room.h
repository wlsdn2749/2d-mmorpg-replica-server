#pragma once

#include "JobQueue.h"
#include "Player.h"
#include "GameSession.h"
#include "MapData.h"
#include <optional>

/*------------------------
		Room (Base)
------------------------*/

using PlayerId = int32; // 

class Room : public JobQueue
{
public:
	struct Cfg {
		int		id					= 0; // Room Id
		string	name				= "Room"; 
		uint32	tickMs				= 50;
		size_t	capacity			= 200; // 최대 수용량
		int		moveCooldownTicks	= 5;
		int		rotateCooldownTicks	= 5;
	};

	explicit Room(Cfg cfg);
	explicit Room(Cfg cfg, std::shared_ptr<MapData> map);
	virtual ~Room();
	void StartTicking(); // 생성 직후 호출

protected:
	Cfg _cfg;
	shared_ptr<const MapData> _map;
	int _tick = 0;

	struct PendingMove {
		bool has = false;
		Protocol::Vector2Info clickWorldPos;	// 최신 이동 지점
		int clientSeq = 0; // 보정 시 필요 
		int recvTick = 0;
	};

	struct PState {
		PendingMove pending;
		int lastActionTick = -9999; // 마지막 회전/이동 시도 틱
		int failedMoves = 0; // 실패한 이동 수 ?
	};
	std::unordered_map<PlayerId, PlayerRef> _players; // PlayerId는 DB의 CharacterId
	std::unordered_map<PlayerId, PState> _pstates;

	void ProcessMovesTick(); // 이번 틱에 요청이 있는 플레이어만 처리
	void SendMoveAck(PlayerRef p, int clientSeq, Protocol::EMoveResult result);


/*-----------------
	Meta Datas
-----------------*/
public:
	int RoomId() const noexcept { return _cfg.id; }
	const std::string& Name() const noexcept { return _cfg.name; }
	size_t PlayerCount() const noexcept { return _players.size(); }
	size_t Capacity() const noexcept { return _cfg.capacity; }
	int moveCooldownTicks() const noexcept {return _cfg.moveCooldownTicks; }
	int rotateCooldownTicks() const noexcept {return _cfg.rotateCooldownTicks;}

/*----------------------------
	Room Specifics Utils
----------------------------*/
public:
	shared_ptr<Room> SharedThis() {
		return std::static_pointer_cast<Room>(shared_from_this());
	}
	PlayerRef FindPlayer(PlayerId pid);
	Protocol::EDirection DecideFacing(const PlayerRef& p, const Protocol::Vector2Info& clickWorldPos);

	std::unordered_map<PlayerId, PlayerRef> Players() {return _players;}

	void RemovePlayerInternal(int playerId, std::string_view reason);
	void AddPlayerInternal(PlayerRef p, SpawnPoint spawn, Protocol::EDirection dir);
	Protocol::S_PlayerList BuildPlayerListSnapshot(const PlayerRef& forPlayer, bool includeSelf=true) const;

	virtual std::optional<SpawnPoint> ResolveSpawn(int portalId) const; // 스폰
/*--------------------------------------------
	외부 진입점 [Room::Async()] 형태로 호출
	(IOCP/패킷 스레드에서 호출)
--------------------------------------------*/
public:
	void Enter(PlayerRef player); // 반드시 Room::DoAsync() 형태로 호출 해야함
	void Leave(PlayerRef player);
	void ChangeRoomBegin(const PlayerRef& p, const PortalLink& link);
	void OnRecvMoveReq(PlayerRef p, const Protocol::C_PlayerMoveRequest& req);
	void ChangeRoomReady(const PlayerRef& p, const Protocol::C_ChangeRoomReady& pkt);
	virtual void OnRecvAttackReq(const PlayerRef& p, const Protocol::C_PlayerAttackRequest& pkt) {}

/*--------------------------------------------
	BroadCast
--------------------------------------------*/
public:
	void Broadcast(const SendBufferRef& pkt, PlayerId except = -1);
	void BroadcastEnter(const PlayerRef& newcomer);
	void BroadcastLeave(const PlayerRef& leaver);

/*-----------------
	Room Tick
-----------------*/
protected:
	virtual bool CanEnterTile(int nx, int ny) const; // 벽/충돌/맵
	virtual void ReserveTile(int nx, int ny); // 이동 예약
	virtual bool IsTileReserved(int nx, int ny) const; // Default = false
	virtual void OnPlayerMoved(const PlayerRef& p, int ox, int oy); // 좌표로 이동한 후 
	virtual void StartTick(); //  최초 한 번만 호출
	virtual void OnRoomTick(); // 매 Tick마다 호출

private:
	void OnTickTimer();

protected:
	// 파생 훅
	virtual void OnEnter(const PlayerRef&) {}
	virtual void OnLeave(const PlayerRef&) {}
	virtual void OnPlayerHpChanged(int playerId, int newHp);
	virtual void OnPlayerDeath(int playerId, int killerMonsterId);


private:
	// 예약 버퍼(간단 구현: 이번 틱 동안만 유효)
	mutable std::unordered_set<int64_t> _reserved; // (nx<<32)|ny
};

