#pragma once
#include "Protocol.pb.h"

static inline void ForwardTile(int x, int y, Protocol::EDirection dir, int& outX, int& outY)
{ 
    outX = x; outY = y;
    switch (static_cast<Protocol::EDirection>(dir)) {
    case Protocol::EDirection::DIR_UP:    --outY; break;
    case Protocol::EDirection::DIR_DOWN:  ++outY; break;
    case Protocol::EDirection::DIR_LEFT:  --outX; break;
    case Protocol::EDirection::DIR_RIGHT: ++outX; break;
    default: break;
    }
}