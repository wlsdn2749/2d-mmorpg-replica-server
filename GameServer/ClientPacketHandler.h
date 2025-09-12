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
	PKT_C_DeleteCharacterRequest = 6,
	PKT_S_DeleteCharacterReply = 7,
	PKT_C_EnterGame = 8,
	PKT_S_EnterGame = 9,
	PKT_S_PlayerList = 10,
	PKT_S_BroadcastPlayerEnter = 11,
	PKT_C_LeaveGame = 12,
	PKT_S_LeaveGame = 13,
	PKT_S_BroadcastPlayerLeave = 14,
	PKT_C_PlayerMoveRequest = 15,
	PKT_S_PlayerMoveReply = 16,
	PKT_S_BroadcastPlayerMove = 17,
	PKT_S_ChangeRoomBegin = 18,
	PKT_C_ChangeRoomReady = 19,
	PKT_S_ChangeRoomCommit = 20,
	PKT_S_SpawnMonster = 21,
	PKT_S_DespawnMonster = 22,
	PKT_S_BroadcastMonsterMove = 23,
	PKT_S_BroadcastMonsterAttack = 24,
	PKT_S_BroadcastMonsterDeath = 25,
	PKT_C_PlayerAttackRequest = 26,
	PKT_S_BroadcastPlayerAttack = 27,
	PKT_C_InventoryRequest = 28,
	PKT_S_InventoryReply = 29,
	PKT_C_ItemUseRequest = 30,
	PKT_S_ItemUseReply = 31,
	PKT_S_InventoryUpdate = 32,
	PKT_S_SystemMessage = 33,
	PKT_S_MonsterList = 34,
	PKT_C_NpcInteractRequest = 35,
	PKT_S_NpcInteractReply = 36,
	PKT_S_NpcShopOpen = 37,
	PKT_C_NpcShopBuyRequest = 38,
	PKT_S_NpcShopBuyReply = 39,
	PKT_S_PlayerStat = 40,
	PKT_S_BroadcastPlayerTryAttack = 41,

};

// Custom Handler : 직접 컨텐츠 작업자가 CPP를 만들어야함

bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_JwtLoginRequest(PacketSessionRef& session, Protocol::C_JwtLoginRequest& pkt);
bool Handle_C_CreateCharacterRequest(PacketSessionRef& session, Protocol::C_CreateCharacterRequest& pkt);
bool Handle_C_CharacterListRequest(PacketSessionRef& session, Protocol::C_CharacterListRequest& pkt);
bool Handle_C_DeleteCharacterRequest(PacketSessionRef& session, Protocol::C_DeleteCharacterRequest& pkt);
bool Handle_C_EnterGame(PacketSessionRef& session, Protocol::C_EnterGame& pkt);
bool Handle_C_LeaveGame(PacketSessionRef& session, Protocol::C_LeaveGame& pkt);
bool Handle_C_PlayerMoveRequest(PacketSessionRef& session, Protocol::C_PlayerMoveRequest& pkt);
bool Handle_C_ChangeRoomReady(PacketSessionRef& session, Protocol::C_ChangeRoomReady& pkt);
bool Handle_C_PlayerAttackRequest(PacketSessionRef& session, Protocol::C_PlayerAttackRequest& pkt);
bool Handle_C_InventoryRequest(PacketSessionRef& session, Protocol::C_InventoryRequest& pkt);
bool Handle_C_ItemUseRequest(PacketSessionRef& session, Protocol::C_ItemUseRequest& pkt);
bool Handle_C_NpcInteractRequest(PacketSessionRef& session, Protocol::C_NpcInteractRequest& pkt);
bool Handle_C_NpcShopBuyRequest(PacketSessionRef& session, Protocol::C_NpcShopBuyRequest& pkt);

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
		GPacketHandler[PKT_C_DeleteCharacterRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_DeleteCharacterRequest>(Handle_C_DeleteCharacterRequest, session, buffer, len); };
		GPacketHandler[PKT_C_EnterGame] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_EnterGame>(Handle_C_EnterGame, session, buffer, len); };
		GPacketHandler[PKT_C_LeaveGame] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_LeaveGame>(Handle_C_LeaveGame, session, buffer, len); };
		GPacketHandler[PKT_C_PlayerMoveRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_PlayerMoveRequest>(Handle_C_PlayerMoveRequest, session, buffer, len); };
		GPacketHandler[PKT_C_ChangeRoomReady] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_ChangeRoomReady>(Handle_C_ChangeRoomReady, session, buffer, len); };
		GPacketHandler[PKT_C_PlayerAttackRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_PlayerAttackRequest>(Handle_C_PlayerAttackRequest, session, buffer, len); };
		GPacketHandler[PKT_C_InventoryRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_InventoryRequest>(Handle_C_InventoryRequest, session, buffer, len); };
		GPacketHandler[PKT_C_ItemUseRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_ItemUseRequest>(Handle_C_ItemUseRequest, session, buffer, len); };
		GPacketHandler[PKT_C_NpcInteractRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_NpcInteractRequest>(Handle_C_NpcInteractRequest, session, buffer, len); };
		GPacketHandler[PKT_C_NpcShopBuyRequest] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::C_NpcShopBuyRequest>(Handle_C_NpcShopBuyRequest, session, buffer, len); };
		
	}
	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_JwtLoginReply& pkt) { return MakeSendBuffer(pkt, PKT_S_JwtLoginReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_CreateCharacterReply& pkt) { return MakeSendBuffer(pkt, PKT_S_CreateCharacterReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_CharacterListReply& pkt) { return MakeSendBuffer(pkt, PKT_S_CharacterListReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DeleteCharacterReply& pkt) { return MakeSendBuffer(pkt, PKT_S_DeleteCharacterReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_EnterGame& pkt) { return MakeSendBuffer(pkt, PKT_S_EnterGame); };
	static SendBufferRef MakeSendBuffer(Protocol::S_PlayerList& pkt) { return MakeSendBuffer(pkt, PKT_S_PlayerList); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastPlayerEnter& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastPlayerEnter); };
	static SendBufferRef MakeSendBuffer(Protocol::S_LeaveGame& pkt) { return MakeSendBuffer(pkt, PKT_S_LeaveGame); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastPlayerLeave& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastPlayerLeave); };
	static SendBufferRef MakeSendBuffer(Protocol::S_PlayerMoveReply& pkt) { return MakeSendBuffer(pkt, PKT_S_PlayerMoveReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastPlayerMove& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastPlayerMove); };
	static SendBufferRef MakeSendBuffer(Protocol::S_ChangeRoomBegin& pkt) { return MakeSendBuffer(pkt, PKT_S_ChangeRoomBegin); };
	static SendBufferRef MakeSendBuffer(Protocol::S_ChangeRoomCommit& pkt) { return MakeSendBuffer(pkt, PKT_S_ChangeRoomCommit); };
	static SendBufferRef MakeSendBuffer(Protocol::S_SpawnMonster& pkt) { return MakeSendBuffer(pkt, PKT_S_SpawnMonster); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DespawnMonster& pkt) { return MakeSendBuffer(pkt, PKT_S_DespawnMonster); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastMonsterMove& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastMonsterMove); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastMonsterAttack& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastMonsterAttack); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastMonsterDeath& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastMonsterDeath); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastPlayerAttack& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastPlayerAttack); };
	static SendBufferRef MakeSendBuffer(Protocol::S_InventoryReply& pkt) { return MakeSendBuffer(pkt, PKT_S_InventoryReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_ItemUseReply& pkt) { return MakeSendBuffer(pkt, PKT_S_ItemUseReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_InventoryUpdate& pkt) { return MakeSendBuffer(pkt, PKT_S_InventoryUpdate); };
	static SendBufferRef MakeSendBuffer(Protocol::S_SystemMessage& pkt) { return MakeSendBuffer(pkt, PKT_S_SystemMessage); };
	static SendBufferRef MakeSendBuffer(Protocol::S_MonsterList& pkt) { return MakeSendBuffer(pkt, PKT_S_MonsterList); };
	static SendBufferRef MakeSendBuffer(Protocol::S_NpcInteractReply& pkt) { return MakeSendBuffer(pkt, PKT_S_NpcInteractReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_NpcShopOpen& pkt) { return MakeSendBuffer(pkt, PKT_S_NpcShopOpen); };
	static SendBufferRef MakeSendBuffer(Protocol::S_NpcShopBuyReply& pkt) { return MakeSendBuffer(pkt, PKT_S_NpcShopBuyReply); };
	static SendBufferRef MakeSendBuffer(Protocol::S_PlayerStat& pkt) { return MakeSendBuffer(pkt, PKT_S_PlayerStat); };
	static SendBufferRef MakeSendBuffer(Protocol::S_BroadcastPlayerTryAttack& pkt) { return MakeSendBuffer(pkt, PKT_S_BroadcastPlayerTryAttack); };

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