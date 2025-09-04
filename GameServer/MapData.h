#pragma once

// 이 포탈을 타면 어디로 가는가?

enum class ESpawnType : int8
{
	PLAYER_SPAWN,
	PORTAL
};

struct PortalLink
{
	int srcPortalId;
	int dstMapId;
	int dstPortalId;
};

struct SpawnPoint
{
	ESpawnType spawnType { ESpawnType::PLAYER_SPAWN };
	int x { 0 };
	int y { 0 };
};

struct MapData
{
/* Map Tiles*/
	int minX = 0, maxX = 0;
	int minY = 0, maxY = 0;
	int width = 0, height = 0;
	int mapId = 0;

	int version;
	float cellSizeX, cellSizeY;
	int originX, originY;
	int worldTopLeftX, worldTopLeftY;
	int worldBottomRightX, worldBottomRightY;

	// 1=통과, 0=충돌
	std::vector<uint8_t> tiles;
	std::unordered_map<int, SpawnPoint> spawnPoints; // portalId -> (x,y)

	inline bool InBounds(int x, int y) const
	{
		return (minX <= x && x <= maxX && minY <= y && y <= maxY);
	}


	// (5, -5) 체크 하려면?
	// (5, 5) 체크해야함

	inline int Index(int x, int y) const
	{
		// 전제: 이미 InBounds 체크 완료
		const int col = x - minX;     // 0..width-1
		const int row = maxY - y;     // -값이 항상 들어오므로 이걸 반전해야함
		return row * width + col;
	}

	inline bool IsBlocked(int x, int y) const
	{
		if (!InBounds(x, y)) return true; // 경계 밖은 차단
		return tiles[Index(x, y)] == 0; // 0이면 못감, 1,2이면 감
	}

	const SpawnPoint* GetSpawnPoint(int portalId) const
	{
		auto it = spawnPoints.find(portalId);
		return (it == spawnPoints.end()) ? nullptr : &it->second;
	}

	static std::shared_ptr<MapData> LoadMapFromJsonFile(const std::string& path);

private:
	static ESpawnType GetESpawnType(string type);

public:
/* Map Portals */
	std::unordered_map<uint64_t, int> portalIdByTile; // (x,y) -> portalId
	std::unordered_map<int, PortalLink> portalLinks; // portalId -> PortalData

	static inline long long TileKey(int x, int y)
	{
		return ((uint64_t)(uint32_t)x << 32) | (uint64_t)(uint32_t)y;
	}
	
	const PortalLink* GetPortalAt(int x, int y) const {
		auto it = portalIdByTile.find(TileKey(x, y));
		if (it == portalIdByTile.end()) return nullptr;
		auto it2 = portalLinks.find(it->second);
		return (it2 == portalLinks.end()) ? nullptr : &it2->second;
	}

	void MapTileToPortal(ESpawnType spawnType, int x, int y, int portalId) {
		portalIdByTile[TileKey(x, y)] = portalId;
		spawnPoints[portalId] = SpawnPoint{ spawnType, x, y };
	}
	void DefinePortal(PortalLink link) {
		portalLinks[link.srcPortalId] = std::move(link);
	}


};

