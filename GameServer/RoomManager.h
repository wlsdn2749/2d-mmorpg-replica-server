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

private:
	std::unordered_map<int, RoomRef> _rooms;
};

