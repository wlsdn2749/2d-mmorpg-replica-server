#include "pch.h"
#include "ClientPacketHandler.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return true;
}

bool Handle_C_JWT_LOGIN_REQUEST(PacketSessionRef& session, Protocol::C_JWT_LOGIN_REQUEST& pkt)
{
	return true;
}

bool Handle_C_CREATE_CHARACTER_REQUEST(PacketSessionRef& session, Protocol::C_CREATE_CHARACTER_REQUEST& pkt)
{
	return true;
}
