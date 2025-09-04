#pragma once
struct EntityCore
{
	EntityId	id{};
	EntityKind	kind{};
	Pos2		pos{};
	Protocol::EDirection	dir{ Protocol::EDirection::DIR_DOWN};
};

