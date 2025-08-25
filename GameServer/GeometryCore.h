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

static inline Pos2 DirToDelta4(Protocol::EDirection d) {
	switch (d) {
	case Protocol::EDirection::DIR_UP:    return { 0,-1 };
	case Protocol::EDirection::DIR_RIGHT: return { 1,0 };
	case Protocol::EDirection::DIR_DOWN:  return { 0,1 };
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