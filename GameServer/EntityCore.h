#pragma once

#include "TypeCore.h"
#include "Protocol.pb.h"
#include "GeometryCore.h"

struct EntityCore
{
	EntityId	id{};
	EntityKind	kind{};
	Pos2		pos{};
	Protocol::EDirection	dir{ Protocol::EDirection::DIR_DOWN};
};

