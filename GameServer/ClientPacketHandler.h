#pragma once
#include "Protocol.pb.h"


using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX]; // 65535만큼의 배열 개수

enum : uint16
{
	PKT_C_JwtLoginRequest = 0,
	PKT_S_JwtLoginReply = 1,
	PKT_C_CreateCharacterRequest = 2,
	PKT_S_CreateCharacterReply = 3,
	PKT_C_CharacterListRequest = 4,
	PKT_S_CharacterListReply = 5,
	PKT_C_EnterGame = 6,
	PKT_S_EnterGame = 7,
	PKT_S_PlayerList = 8,
	PKT_S_BroadcastPlayerEnter = 9,
	PKT_C_LeaveGame = 10,
	PKT_S_BroadcastPlayerLeave = 11,
	PKT_C_PlayerMoveRequest = 12,
	PKT_S_PlayerMoveReply = 13,
	PKT_S_BroadcastPlayerMove = 14,

};

// Custom Handler : 직접 컨텐츠 작업자가 CPP를 만들어야함

bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_JwtLoginRequest(PacketSessionRef& session, Protocol::C_JwtLoginRequest& pkt);
bool Handle_C_CreateCharacterRequest(PacketSessionRef& session, Protocol::C_CreateCharacterRequest& pkt);
bool Handle_C_CharacterListRequest(PacketSessionRef& session, Protocol::C_CharacterListRequest& pkt);
bool Handle_C_EnterGame(PacketSessionRef& session, Protocol::C_EnterGame& pkt);
bool Handle_C_LeaveGame(PacketSessionRef& session, Protocol::C_LeaveGame& pkt);
bool Handle_C_PlayerMoveRequest(PacketSessionRef& session, Protocol::C_PlayerMoveRequest& pkt);

class ClientPacketHandler
{
public:

	static void init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
		{
			GPacketHandler[i] = Handle_Invalid;
		}
		GPacketHandler[PKT_C_JwtLoginRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_JwtLoginRequest>(Handle_C_JwtLoginRequest, session, buffer, len); };
		GPacketHandler[PKT_C_CreateCharacterRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_CreateCharacterRequest>(Handle_C_CreateCharacterRequest, session, buffer, len); };
		GPacketHandler[PKT_C_CharacterListRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_CharacterListRequest>(Handle_C_CharacterListRequest, session, buffer, len); };
		GPacketHandler[PKT_C_EnterGame] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_EnterGame>(Handle_C_EnterGame, session, buffer, len); };
		GPacketHandler[PKT_C_LeaveGame] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_LeaveGame>(Handle_C_LeaveGame, session, buffer, len); };
		GPacketHandler[PKT_C_PlayerMoveRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_PlayerMoveRequest>(Handle_C_PlayerMoveRequest, session, buffer, len); };
		
	}
	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_JwtLoginReply& pkt) { return MakeSendBuffer(pkt, PKT_S_JwtLoginReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_CreateCharacterReply& pkt) { return MakeSendBuffer(pkt, PKT_S_CreateCharacterReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_CharacterListReply& pkt) { return MakeSendBuffer(pkt, PKT_S_CharacterListReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_EnterGame& pkt) { return MakeSendBuffer(pkt, PKT_S_EnterGame); };
	static SendBufferRef MakeSendBuffer(Protocol::S_PlayerList& pkt) { return MakeSendBuffer(pkt, PKT_S_PlayerList); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastPlayerEnter& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastPlayerEnter); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastPlayerLeave& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastPlayerLeave); };
	static SendBufferRef MakeSendBuffer(Protocol::S_PlayerMoveReply& pkt) { return MakeSendBuffer(pkt, PKT_S_PlayerMoveReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastPlayerMove& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastPlayerMove); };

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