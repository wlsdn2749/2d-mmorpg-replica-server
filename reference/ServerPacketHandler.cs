using DummyClient;
using ServerCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Google.Protobuf;
using Protocol;
using UnityEngine;

namespace Packet
{
    
    public class ServerPacketHandler : MonoBehaviour
    {
        public static void HANDLE_INVALID(PacketSession session, IMessage packet)
        {
            Debug.Log("[패킷 처리] INVALID PACKET (알수 없는 패킷 ID 요청!)");
        }
    
        public static void HANDLE_S_REGISTER(PacketSession session, S_REGISTER packet)
        {
            S_REGISTER pkt = packet as S_REGISTER;
            ServerSession serverSession = session as ServerSession;
            PlayerManager.Instance.Register(pkt);
        }
        
        // (CLIENT) C_LOGIN -> (SERVER) S_LOGIN에 응답하기 위한 패킷
        public static void HANDLE_S_LOGIN(PacketSession session, S_LOGIN packet)
        {
            S_LOGIN pkt = packet as S_LOGIN;
            ServerSession serverSession = session as ServerSession;
            PlayerManager.Instance.Login(pkt);

        }
        public static void HANDLE_S_ENTER_GAME(PacketSession session, S_ENTER_GAME packet)
        {
            Debug.Log("[패킷 처리] HANDLE_S_ENTER_GAME PACKET (SERVER가 S_ENTER_GAME 요청을 보냈음)");
        }
        public static void HANDLE_S_BROADCAST_CHAT(PacketSession session, S_BROADCAST_CHAT packet)
        {
            S_BROADCAST_CHAT pkt = packet as S_BROADCAST_CHAT;
            ServerSession serverSession = session as ServerSession;
            
            // UnityEngine.Debug.Log($"[Server's Broadcast] {pkt.PlayerId}: {pkt.Msg}");
            UI.ChatUIManager.Instance.AddMessages(pkt);
        }
        public static void HANDLE_S_PLAYERLIST(PacketSession session, S_PLAYERLIST packet)
        {
            S_PLAYERLIST pkt = packet as S_PLAYERLIST;
            ServerSession serverSession = session as ServerSession;
            Debug.Log("목록 받기 완료");
            PlayerManager.Instance.Add(pkt);
        }
        public static void HANDLE_S_BROADCAST_ENTER_GAME(PacketSession session, S_BROADCAST_ENTER_GAME packet)
        {
            S_BROADCAST_ENTER_GAME pkt = packet as S_BROADCAST_ENTER_GAME;
            ServerSession serverSession = session as ServerSession;
            PlayerManager.Instance.EnterGame(pkt);
        }
        public static void HANDLE_S_BROADCAST_MOVE(PacketSession session, S_BROADCAST_MOVE packet)
        {
            S_BROADCAST_MOVE pkt = packet as S_BROADCAST_MOVE;
            ServerSession serverSession = session as ServerSession;
            PlayerManager.Instance.Move(pkt);
        }
        public static void HANDLE_S_BROADCAST_LEAVE_GAME(PacketSession session, S_BROADCAST_LEAVE_GAME packet)
        {
            S_BROADCAST_LEAVE_GAME pkt = packet as S_BROADCAST_LEAVE_GAME;
            ServerSession serverSession = session as ServerSession;
            PlayerManager.Instance.LeaveGame(pkt);
        }
        
        public static void HANDLE_S_RTT(PacketSession session, S_RTT packet)
        {
            S_RTT pkt = packet as S_RTT;
            ServerSession serverSession = session as ServerSession;
            
            // 현재 클라이언트 시간을 마이크로초 단위로 측정
            var now = DateTime.UtcNow;
            var ticks = now.Ticks;
            var currentMicroseconds = ticks / 10; // Ticks를 마이크로초로 변환 (1 tick = 100 nanoseconds)
            
            // RTT 계산 (현재 시간 - 원래 클라이언트 전송 시간)
            ulong originalClientTime = pkt.ClientTime;
            ulong currentTime = (ulong)currentMicroseconds;
            
            // RTT = 현재 시간 - 원래 전송 시간 (마이크로초)
            double rttMicroseconds = (double)(currentTime - originalClientTime);
            double rttMs = rttMicroseconds / 1000.0; // 마이크로초를 밀리초로 변환
            
            // 비정상적인 값 필터링 (음수이거나 너무 큰 값)
            if (rttMs < 0 || rttMs > 10000) // 10초 이상은 비정상
            {
                UnityEngine.Debug.LogWarning($"[RTT 측정] 비정상적인 RTT 값: {rttMs:F2}ms - 무시됨");
                return;
            }
            
            PlayerManager.Instance.MyPlayer?.SetRtt(rttMs);
            // UnityEngine.Debug.Log($"[RTT 측정] {rttMs:F2}ms");
        }
    }
}