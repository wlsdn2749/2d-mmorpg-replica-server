#pragma once
#include <unordered_map>
#include "Room.h"

using RoomRef = shared_ptr<Room>;

class RoomManager
{
#pragma region Meyers Singleton
public:
	static RoomManager& Instance()
	{
		static RoomManager roomManager;
		return roomManager;
	}

	RoomManager(const RoomManager&) = delete;
	RoomManager& operator=(const RoomManager&) = delete;
private:
	RoomManager() = default;
	~RoomManager() = default;

#pragma endregion

public:
	void Add(RoomRef r) 
	{
		_mapNameToIds[r->RoomName()] = r->RoomId();
		_rooms[r->RoomId()] = std::move(r); // r을 Move하기 때문에 mapNametoIds가 먼저
	}

public:
	// Execute DoAsync for All Rooms 

	// Lambda or Func 
	template<typename F>
	void DoAsyncForAllRooms(F&& f)
	{
		for (auto& [roomId, room] : _rooms)
		{
			if (room)
			{
				room->DoAsync(std::forward<F>(f));
			}
		}
	}

	// 멤버함수 포인터 버전
	template<typename T, typename Ret, typename... FArgs, typename... CallArgs>
	void DoAsyncForAllRooms(Ret(T::* memFunc)(FArgs...), CallArgs&&... callArgs)
	{
		for (auto& [roomId, room] : _rooms)
		{
			if (room)
			{
				room->DoAsync(memFunc, std::forward<CallArgs>(callArgs)...);
			}
		}
	}

	// 멤버함수 포인터 Const 버전
	template<typename T, typename Ret, typename... FArgs, typename... CallArgs>
	void DoAsyncForAllRooms(Ret(T::* memFunc)(FArgs...) const, CallArgs&&... callArgs)
	{
		for (auto& [roomId, room] : _rooms) {
			if (room) {
				room->DoAsync(memFunc, std::forward<CallArgs>(callArgs)...);
			}
		}
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
public:
	// Search PlayerId -> PlayerRef
	PlayerRef FindPlayerInAllRooms(int32 pid)
	{
		PlayerRef result = nullptr;
		for (const auto& [roomId, room] : _rooms)
		{
			if (room)
			{
				if (auto player = room->FindPlayer(pid))
				{
					result = player;
					break;
				}
			}
		}
		return result;
	}

private:
	std::unordered_map<string, int> _mapNameToIds;
	std::unordered_map<int, RoomRef> _rooms; // Id -> RoomRef
};

