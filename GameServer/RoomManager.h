#pragma once
#include <unordered_map>
#include "Room.h"

using RoomRef = shared_ptr<Room>;

class RoomManager
{
public:
	static RoomManager& Instance()
	{
		static RoomManager roomManager; 
		return roomManager;
	}

	void Add(RoomRef r) {_rooms[r->RoomId()] = std::move(r); }
	RoomRef Find(int id) {
		auto it = _rooms.find(id);
		return (it == _rooms.end()) ? nullptr : it->second;
	}

public:
	// Helper Function
	// (ERegion) -> RoomId
	inline int GetRoomIdByRegion(Protocol::ERegion region)
	{
		if(region == Protocol::ERegion::REGION_GO) return 1;
		else if (region == Protocol::ERegion::REGION_BACK) return 2;
	}

private:
	std::unordered_map<int, RoomRef> _rooms;
};

