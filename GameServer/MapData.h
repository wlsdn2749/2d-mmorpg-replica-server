#pragma once

// 이 포탈을 타면 어디로 가는가?
struct PortalLink
{
	int srcPortalId;
	int dstMapId;
	int dstPortalId;
};

struct SpawnPoint
{
	int x;
	int y;
};

struct MapData
{
/* Map Tiles*/
	int minX = 0, maxX = 0;
	int minY = 0, maxY = 0;
	int width = 0, height = 0;
	int mapId = 0;

	// 0=통과, 1=충돌
	std::vector<uint8_t> tiles;
	std::unordered_map<int, SpawnPoint> spawnPoints; // portalId -> (x,y)

	inline bool InBounds(int x, int y) const
	{
		return (x >= minX && x <= maxX && y >= minY && y <= maxY);
	}

	inline int Index(int x, int y) const
	{
		// 전제: 이미 InBounds 체크 완료
		const int col = x - minX;     // 0..width-1
		const int row = y - minY;     // 0..height-1
		return row * width + col;
	}

	inline bool IsBlocked(int x, int y) const
	{
		if (!InBounds(x, y)) return true; // 경계 밖은 차단
		return tiles[Index(x, y)] != 0;
	}

	const SpawnPoint* GetSpawnPoint(int portalId) const
	{
		auto it = spawnPoints.find(portalId);
		return (it == spawnPoints.end()) ? nullptr : &it->second;
	}

	static std::shared_ptr<MapData> FromFile(const std::string& path);

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

	void MapTileToPortal(int x, int y, int portalId) {
		portalIdByTile[TileKey(x, y)] = portalId;
		spawnPoints[portalId] = SpawnPoint{ x, y };
	}
	void DefinePortal(PortalLink link) {
		portalLinks[link.srcPortalId] = std::move(link);
	}


};

