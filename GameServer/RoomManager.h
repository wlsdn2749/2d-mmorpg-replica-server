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

	void Add(RoomRef r) 
	{
		_mapNameToIds[r->RoomName()] = r->RoomId();
		_rooms[r->RoomId()] = std::move(r); // r을 Move하기 때문에 mapNametoIds가 먼저
	}


public:
	// Helper Function
	
	// RoomId -> RoomRef
	RoomRef Find(int id) 
	{
		auto it = _rooms.find(id);
		return (it == _rooms.end()) ? nullptr : it->second;
	}

	// Name -> RoomId
	int GetRoomIdByName(const string& name)
	{
		auto it = _mapNameToIds.find(name);
		return (it == _mapNameToIds.end()) ? -1 : it->second;
	}

	// ERegion -> Name
	string GetNameByRegion(const Protocol::ERegion region)
	{
		if(region == Protocol::ERegion::REGION_GO) return "Goguryeo"; // TODO ShardBoot.cpp HARDCODE
		else if(region == Protocol::ERegion::REGION_BACK) return "Baekje";
		else if(region == Protocol::ERegion::REGION_NONE) return "Goguryeo";
	}

	int GetRoomIdByRegion(const Protocol::ERegion region)
	{
		return GetRoomIdByName(GetNameByRegion(region));
	}

private:
	std::unordered_map<string, int> _mapNameToIds;
	std::unordered_map<int, RoomRef> _rooms; // Id -> RoomRef
};

