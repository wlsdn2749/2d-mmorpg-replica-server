#pragma once
#include "Protocol.pb.h"


using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX]; // 65535만큼의 배열 개수

enum : uint16
{
	PKT_C_JWT_LOGIN_REQUEST = 1000,
	PKT_S_JWT_LOGIN_REPLY = 1001,
	PKT_C_CREATE_CHARACTER_REQUEST = 1002,
	PKT_S_CREATE_CHARACTER_REPLY = 1003,

};

// Custom Handler : 직접 컨텐츠 작업자가 CPP를 만들어야함

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_JWT_LOGIN_REQUEST(PacketSessionRef& session, Protocol::C_JWT_LOGIN_REQUEST& pkt);
bool Handle_C_CREATE_CHARACTER_REQUEST(PacketSessionRef& session, Protocol::C_CREATE_CHARACTER_REQUEST& pkt);

class ClientPacketHandler
{
public:

	static void init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
		{
			GPacketHandler[i] = Handle_INVALID;
		}
		GPacketHandler[PKT_C_JWT_LOGIN_REQUEST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_JWT_LOGIN_REQUEST>(Handle_C_JWT_LOGIN_REQUEST, session, buffer, len); };
		GPacketHandler[PKT_C_CREATE_CHARACTER_REQUEST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_CREATE_CHARACTER_REQUEST>(Handle_C_CREATE_CHARACTER_REQUEST, session, buffer, len); };
		
	}
	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_JWT_LOGIN_REPLY& pkt) { return MakeSendBuffer(pkt, PKT_S_JWT_LOGIN_REPLY); };
	static SendBufferRef MakeSendBuffer(Protocol::S_CREATE_CHARACTER_REPLY& pkt) { return MakeSendBuffer(pkt, PKT_S_CREATE_CHARACTER_REPLY); };

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong()); // byte로 환산하면 얼마인가?

		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;

		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));


		sendBuffer->Close(packetSize);
		return sendBuffer;
	}

};