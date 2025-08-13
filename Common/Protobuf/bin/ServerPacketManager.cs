using ServerCore;
using System;
using System.Collections.Generic;
using Google.Protobuf;
using Google.Protobuf.Protocol;

namespace Packet
{
    public enum PacketID : ushort
    {
	    PKT_C_JwtLoginRequest = 1000,
	    PKT_S_JwtLoginReply = 1001,
	    PKT_C_CreateCharacterRequest = 1002,
	    PKT_S_CreateCharacterReply = 1003,
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

        void Register()
        {
            for (int i = 0; i < UInt16.MaxValue + 1; i++)
            {
                _packetHandlers[i] = ServerPacketHandler.HANDLE_INVALID;
            }
            RegisterHandler((ushort)PacketID.PKT_S_JwtLoginReply, ServerPacketHandler.HANDLE_S_JwtLoginReply, S_JwtLoginReply.Parser);
            RegisterHandler((ushort)PacketID.PKT_S_CreateCharacterReply, ServerPacketHandler.HANDLE_S_CreateCharacterReply, S_CreateCharacterReply.Parser);
            
                  
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