#pragma once
struct MapData
{
	int minX = 0, maxX = 0;
	int minY = 0, maxY = 0;
	int width = 0, height = 0;

	// 0=통과, 1=충돌
	std::vector<uint8_t> tiles;

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

	static std::shared_ptr<const MapData> FromFile(const std::string& path);
};

