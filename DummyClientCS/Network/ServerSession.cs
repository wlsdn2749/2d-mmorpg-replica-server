using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Packet;
using ServerCore;
using Protocol;

namespace DummyClientCS
{
    enum AuthState : ushort
    {
        None = 0, // 로그인 전 상태
        Waiting = 1, 
        Failed = 2, // 실패, 차단
        Unknown = 3,
        Success = 4, // 성공 
        Error = 99, 
    };

    class ServerSession : PacketSession
    {
        /* --------------상태 플래그 --------------*/
        public bool IsLoggedIn { get; private set; } = false;



        /* -------------- Network Packet 송수신 부분 --------------*/

        public override void OnConnected(EndPoint endPoint)
        {
            //Console.WriteLine($"onConnected : {endPoint}");

            // 중복처리 안되있어서 일단 비활성화
            //Protocol.C_REGISTER registerPkt = new C_REGISTER()
            //{
            //    Email = "abc@123d.com",
            //    Pw = "abcde"
            //};
            //var registerSendBuffer = ServerPacketManager.MakeSendBuffer(registerPkt);
            //Send(registerSendBuffer);

            Protocol.C_LOGIN loginPkt = new C_LOGIN()
            {
                Email = "abc@123d.com",
                Pw = "abcde",
                Type = Protocol.PlayerType.Dummy,
            };
            var sendBuffer = ServerPacketManager.MakeSendBuffer(loginPkt);
            Send(sendBuffer);
        }

        public override void OnDisconnected(EndPoint endPoint)
        {
            if (endPoint != null)
                Console.WriteLine($"onDisconnected : {endPoint}");
            else
                Console.WriteLine("onDisconnected : (null endpoint)");
        }

        public override void OnRecvPacket(ArraySegment<byte> buffer)
        {
            ServerPacketManager.Instance.OnRecvPacket(this, buffer);
            ClientPerformanceStats.Instance.OnPacketReceived();
        }

        public override void OnSend(int numOfBytes)
        {
            //Console.WriteLine($"Transfered bytes: {numOfBytes}");
        }
    }
}
