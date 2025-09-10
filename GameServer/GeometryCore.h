#pragma once

class GeometryCore
{
};

struct Pos2 {
	int32 x{};
	int32 y{};

	constexpr Pos2() = default;
	constexpr Pos2(int32 x_, int32 y_) : x(x_), y(y_) {}

	// 연산자
	constexpr bool operator==(const Pos2& o) const { return x == o.x && y == o.y; }
	constexpr bool operator!=(const Pos2& o) const { return !(*this == o); }
	constexpr Pos2 operator+(const Pos2& o) const { return { x + o.x, y + o.y }; }
	constexpr Pos2 operator-(const Pos2& o) const { return { x - o.x, y - o.y }; }

	static inline int32 Manhattan(const Pos2& a, const Pos2& b) {
		return std::abs(a.x - b.x) + std::abs(a.y - b.y);
	}
};

template<>
struct std::hash<Pos2>
{
	std::size_t operator()(const Pos2& pos) const noexcept
	{
		std::size_t x = std::hash<int32>{}(pos.x);
		std::size_t y = std::hash<int32>{}(pos.y);
		return x ^ (y << 1); // or use boost::hash_combine
	}
};

static inline Pos2 DirToDelta4(Protocol::EDirection d) {
	switch (d) {
	case Protocol::EDirection::DIR_UP:    return { 0,1 };
	case Protocol::EDirection::DIR_RIGHT: return { 1,0 };
	case Protocol::EDirection::DIR_DOWN:  return { 0,-1 };
	case Protocol::EDirection::DIR_LEFT:  return { -1,0 };
	}
	return { 0,0 };
}

static inline Protocol::EDirection FaceTo(const Pos2& from, const Pos2& to) {
	const int dx = to.x - from.x;
	const int dy = to.y - from.y;
	if (std::abs(dx) >= std::abs(dy))
		return (dx >= 0) ? Protocol::EDirection::DIR_RIGHT : Protocol::EDirection::DIR_LEFT;
	else
		return (dy >= 0) ? Protocol::EDirection::DIR_DOWN : Protocol::EDirection::DIR_UP;
}

static inline void ForwardTile(int x, int y, Protocol::EDirection dir, int& outX, int& outY)
{
	outX = x; outY = y;
	switch (static_cast<Protocol::EDirection>(dir)) {
	case Protocol::EDirection::DIR_UP:    ++outY; break;
	case Protocol::EDirection::DIR_DOWN:  --outY; break;
	case Protocol::EDirection::DIR_LEFT:  --outX; break;
	case Protocol::EDirection::DIR_RIGHT: ++outX; break;
	default: break;
	}
}

static inline void ForwardTile(const Pos2& pos, Protocol::EDirection dir, Pos2& outPos)
{
	outPos = pos;
	switch (static_cast<Protocol::EDirection>(dir))
	{
	case Protocol::EDirection::DIR_UP:    ++outPos.y; break;
	case Protocol::EDirection::DIR_DOWN:  --outPos.y; break;
	case Protocol::EDirection::DIR_LEFT:  --outPos.x; break;
	case Protocol::EDirection::DIR_RIGHT: ++outPos.x; break;
	default: break;
	}
}
