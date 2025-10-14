using ServerCore;
using System;
using System.Collections.Generic;
using Google.Protobuf;
using Google.Protobuf.Protocol;

namespace Packet
{
    public enum PacketID : ushort
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
	    PKT_S_BroadcastPlayerHpChanged = 42,
	    PKT_S_BroadcastPlayerDeath = 43,
	    PKT_C_PlayerDeathReady = 44,
	    PKT_S_PlayerDeathCommit = 45,
	    PKT_C_PlayerChat = 46,
	    PKT_S_BroadcastPlayerChat = 47,
	    PKT_C_GiveItemRequest = 48,
	    PKT_S_GiveItemReply = 49,
	    PKT_C_PartyInviteRequest = 50,
	    PKT_S_PartyInviteNotify = 51,
	    PKT_S_PartyInviteReply = 52,
	    PKT_C_PartyInviteResponse = 53,
	    PKT_C_PartyLeave = 54,
	    PKT_S_BroadcastPartyUpdate = 55,
	    PKT_C_PartyCreateRequest = 56,
	    PKT_S_PartyCreateReply = 57,
	    PKT_C_PartyJoinRequest = 58,
	    PKT_S_PartyJoinReply = 59,
	    PKT_S_PartyJoinNotify = 60,
	    PKT_C_PartyJoinResponse = 61,
	    PKT_C_PartyList = 62,
	    PKT_S_PartyList = 63,
	    PKT_C_PartyJoinRequestList = 64,
	    PKT_S_PartyJoinRequestList = 65,
	    PKT_C_EquipItemRequest = 66,
	    PKT_S_EquipItemReply = 67,
	    PKT_C_UnequipItemRequest = 68,
	    PKT_S_UnequipItemReply = 69,
	    PKT_C_EquipmentInfoRequest = 70,
	    PKT_S_EquipmentInfoReply = 71,
	    PKT_S_BroadcastPlayerEquipment = 72,
	    PKT_S_NpcList = 73,
	    PKT_C_NpcShopSellRequest = 74,
	    PKT_S_NpcShopSellReply = 75,
    }
    public class ServerPacketManager
    {
        #region Singleton
        static ServerPacketManager _instance = new ServerPacketManager();

        public static ServerPacketManager Instance
        {
            get { return _instance; }
        }
        #endregion

        ServerPacketManager()
        {
            Register();
        }

        // _handler[Protocol.ID]는 그 Protocol.Id를 처리하는 대리자일 것. 아니면 INVALID_로 처리
        // [id] => (Session, IMessage) => Handler
        Action<PacketSession, IMessage>[] _packetHandlers = new Action<PacketSession, IMessage>[ushort.MaxValue + 1];

        // [id] => (data, offset, length) => IMessage
        Dictionary<ushort, Func<byte[], int, int, IMessage>> _messageParsers = new Dictionary<ushort, Func<byte[], int, int, IMessage>>();

        private readonly Dictionary<Type, ushort> _typeToId = new();
        public static ArraySegment<byte> MakeSendBuffer(C_JwtLoginRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_JwtLoginRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_CreateCharacterRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_CreateCharacterRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_CharacterListRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_CharacterListRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_DeleteCharacterRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_DeleteCharacterRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_EnterGame pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_EnterGame);
        public static ArraySegment<byte> MakeSendBuffer(C_LeaveGame pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_LeaveGame);
        public static ArraySegment<byte> MakeSendBuffer(C_PlayerMoveRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PlayerMoveRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_ChangeRoomReady pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_ChangeRoomReady);
        public static ArraySegment<byte> MakeSendBuffer(C_PlayerAttackRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PlayerAttackRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_InventoryRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_InventoryRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_ItemUseRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_ItemUseRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_NpcInteractRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_NpcInteractRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_NpcShopBuyRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_NpcShopBuyRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_PlayerDeathReady pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PlayerDeathReady);
        public static ArraySegment<byte> MakeSendBuffer(C_PlayerChat pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PlayerChat);
        public static ArraySegment<byte> MakeSendBuffer(C_GiveItemRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_GiveItemRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_PartyInviteRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PartyInviteRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_PartyInviteResponse pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PartyInviteResponse);
        public static ArraySegment<byte> MakeSendBuffer(C_PartyLeave pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PartyLeave);
        public static ArraySegment<byte> MakeSendBuffer(C_PartyCreateRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PartyCreateRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_PartyJoinRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PartyJoinRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_PartyJoinResponse pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PartyJoinResponse);
        public static ArraySegment<byte> MakeSendBuffer(C_PartyList pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PartyList);
        public static ArraySegment<byte> MakeSendBuffer(C_PartyJoinRequestList pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_PartyJoinRequestList);
        public static ArraySegment<byte> MakeSendBuffer(C_EquipItemRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_EquipItemRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_UnequipItemRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_UnequipItemRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_EquipmentInfoRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_EquipmentInfoRequest);
        public static ArraySegment<byte> MakeSendBuffer(C_NpcShopSellRequest pkt) => MakeSendBuffer(pkt, (ushort)PacketID.PKT_C_NpcShopSellRequest);

        void Register()
        {
            for (int i = 0; i < UInt16.MaxValue + 1; i++)
            {
                _packetHandlers[i] = ServerPacketHandler.HANDLE_Invalid;
            }
            RegisterHandler((ushort)PacketID.PKT_S_JwtLoginReply, ServerPacketHandler.HANDLE_S_JwtLoginReply, S_JwtLoginReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_CreateCharacterReply, ServerPacketHandler.HANDLE_S_CreateCharacterReply, S_CreateCharacterReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_CharacterListReply, ServerPacketHandler.HANDLE_S_CharacterListReply, S_CharacterListReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_DeleteCharacterReply, ServerPacketHandler.HANDLE_S_DeleteCharacterReply, S_DeleteCharacterReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_EnterGame, ServerPacketHandler.HANDLE_S_EnterGame, S_EnterGame.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PlayerList, ServerPacketHandler.HANDLE_S_PlayerList, S_PlayerList.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerEnter, ServerPacketHandler.HANDLE_S_BroadcastPlayerEnter, S_BroadcastPlayerEnter.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_LeaveGame, ServerPacketHandler.HANDLE_S_LeaveGame, S_LeaveGame.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerLeave, ServerPacketHandler.HANDLE_S_BroadcastPlayerLeave, S_BroadcastPlayerLeave.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PlayerMoveReply, ServerPacketHandler.HANDLE_S_PlayerMoveReply, S_PlayerMoveReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerMove, ServerPacketHandler.HANDLE_S_BroadcastPlayerMove, S_BroadcastPlayerMove.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_ChangeRoomBegin, ServerPacketHandler.HANDLE_S_ChangeRoomBegin, S_ChangeRoomBegin.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_ChangeRoomCommit, ServerPacketHandler.HANDLE_S_ChangeRoomCommit, S_ChangeRoomCommit.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_SpawnMonster, ServerPacketHandler.HANDLE_S_SpawnMonster, S_SpawnMonster.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_DespawnMonster, ServerPacketHandler.HANDLE_S_DespawnMonster, S_DespawnMonster.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastMonsterMove, ServerPacketHandler.HANDLE_S_BroadcastMonsterMove, S_BroadcastMonsterMove.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastMonsterAttack, ServerPacketHandler.HANDLE_S_BroadcastMonsterAttack, S_BroadcastMonsterAttack.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastMonsterDeath, ServerPacketHandler.HANDLE_S_BroadcastMonsterDeath, S_BroadcastMonsterDeath.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerAttack, ServerPacketHandler.HANDLE_S_BroadcastPlayerAttack, S_BroadcastPlayerAttack.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_InventoryReply, ServerPacketHandler.HANDLE_S_InventoryReply, S_InventoryReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_ItemUseReply, ServerPacketHandler.HANDLE_S_ItemUseReply, S_ItemUseReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_InventoryUpdate, ServerPacketHandler.HANDLE_S_InventoryUpdate, S_InventoryUpdate.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_SystemMessage, ServerPacketHandler.HANDLE_S_SystemMessage, S_SystemMessage.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_MonsterList, ServerPacketHandler.HANDLE_S_MonsterList, S_MonsterList.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_NpcInteractReply, ServerPacketHandler.HANDLE_S_NpcInteractReply, S_NpcInteractReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_NpcShopOpen, ServerPacketHandler.HANDLE_S_NpcShopOpen, S_NpcShopOpen.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_NpcShopBuyReply, ServerPacketHandler.HANDLE_S_NpcShopBuyReply, S_NpcShopBuyReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PlayerStat, ServerPacketHandler.HANDLE_S_PlayerStat, S_PlayerStat.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerTryAttack, ServerPacketHandler.HANDLE_S_BroadcastPlayerTryAttack, S_BroadcastPlayerTryAttack.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerHpChanged, ServerPacketHandler.HANDLE_S_BroadcastPlayerHpChanged, S_BroadcastPlayerHpChanged.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerDeath, ServerPacketHandler.HANDLE_S_BroadcastPlayerDeath, S_BroadcastPlayerDeath.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PlayerDeathCommit, ServerPacketHandler.HANDLE_S_PlayerDeathCommit, S_PlayerDeathCommit.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerChat, ServerPacketHandler.HANDLE_S_BroadcastPlayerChat, S_BroadcastPlayerChat.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_GiveItemReply, ServerPacketHandler.HANDLE_S_GiveItemReply, S_GiveItemReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PartyInviteNotify, ServerPacketHandler.HANDLE_S_PartyInviteNotify, S_PartyInviteNotify.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PartyInviteReply, ServerPacketHandler.HANDLE_S_PartyInviteReply, S_PartyInviteReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPartyUpdate, ServerPacketHandler.HANDLE_S_BroadcastPartyUpdate, S_BroadcastPartyUpdate.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PartyCreateReply, ServerPacketHandler.HANDLE_S_PartyCreateReply, S_PartyCreateReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PartyJoinReply, ServerPacketHandler.HANDLE_S_PartyJoinReply, S_PartyJoinReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PartyJoinNotify, ServerPacketHandler.HANDLE_S_PartyJoinNotify, S_PartyJoinNotify.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PartyList, ServerPacketHandler.HANDLE_S_PartyList, S_PartyList.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_PartyJoinRequestList, ServerPacketHandler.HANDLE_S_PartyJoinRequestList, S_PartyJoinRequestList.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_EquipItemReply, ServerPacketHandler.HANDLE_S_EquipItemReply, S_EquipItemReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_UnequipItemReply, ServerPacketHandler.HANDLE_S_UnequipItemReply, S_UnequipItemReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_EquipmentInfoReply, ServerPacketHandler.HANDLE_S_EquipmentInfoReply, S_EquipmentInfoReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_BroadcastPlayerEquipment, ServerPacketHandler.HANDLE_S_BroadcastPlayerEquipment, S_BroadcastPlayerEquipment.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_NpcList, ServerPacketHandler.HANDLE_S_NpcList, S_NpcList.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_NpcShopSellReply, ServerPacketHandler.HANDLE_S_NpcShopSellReply, S_NpcShopSellReply.Parser);
            
                  
        }

        void RegisterHandler<T>(ushort id, Action<PacketSession, T> handler, MessageParser<T> parser) where T : IMessage<T>
        {
            _packetHandlers[id] = (session, packet) => handler(session, (T)packet);

            // IMessage 파서 저장 (OnRecvCallback용)
            _messageParsers[id] = (data, offset, length) => parser.ParseFrom(data, offset, length);

            // type -> id 변환기
            _typeToId[typeof(T)] = id;
        }


        // 서버로 부터 패킷을 받아 처리하는 코드
        // session          :   클라이언트와 서버가 연결되는, 수신 세션
        // buffer           :   수신된 전체 패킷 버퍼
        // OnRecvCallback   :   패킷을 Queueing 하는 부분, 한번에 처리해주는게 더 성능상 좋기 때문
        public void OnRecvPacket(PacketSession session, ArraySegment<byte> buffer, Action<PacketSession, IMessage> OnRecvCallback = null)
        {
            ushort size = BitConverter.ToUInt16(buffer.Array, buffer.Offset);
            ushort id = BitConverter.ToUInt16(buffer.Array, buffer.Offset + 2);
            int protoLen = size - 4;             // Header Size
            int protoOffset = buffer.Offset + 4; // Header Size
            Func<byte[], int, int, IMessage> parser = null;
            if (_messageParsers.TryGetValue(id, out parser))
            {
                IMessage packet = parser.Invoke(buffer.Array, protoOffset, protoLen);
                // 여기서 이미 IMessage형태를 가지고 있어야함.
                if (OnRecvCallback != null)
                {
                    OnRecvCallback.Invoke(session, packet);
                }
                else
                {
                    HandlePacket(session, packet);
                }
            }
        }

        public static ArraySegment<byte> MakeSendBuffer<T>(T pkt, ushort pktId) where T : IMessage<T>
        {
            byte[] body = pkt.ToByteArray();
            ushort bodySize = (ushort)body.Length;
            ushort packetSize = (ushort)(bodySize + 4); // Header 크기 4byte

            byte[] buffer = new byte[packetSize];
            
            // Header 작성
            Array.Copy(BitConverter.GetBytes(packetSize), 0, buffer, 0, 2);           // size
            Array.Copy(BitConverter.GetBytes(pktId), 0, buffer, 2, 2);                // id
            
            // Body 작성
            Array.Copy(body, 0, buffer, 4, body.Length);

            return new ArraySegment<byte>(buffer, 0, packetSize);
        }

        public void HandlePacket(PacketSession session, IMessage packet)
        {
            _packetHandlers[GetPacketId(packet)].Invoke(session, packet);
        }

        ushort GetPacketId(IMessage packet)
        {
            if (_typeToId.TryGetValue(packet.GetType(), out var id))
                return id;

            throw new Exception($"[PacketManager] Unregistered IMessage type: {packet.GetType()}");
        }
    }
}