using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using Unity.VisualScripting.Antlr3.Runtime;
using UnityEngine;
using UnityEngine.TextCore.Text;

namespace Packet
{
    static class NetDebug
    {
        // 내 플레이어ID가 있으면 넣어두면 "ME" 표시 가능 (없으면 -1 유지)
        public static int MyPlayerId = -1;

        public static string DirToStr(EDirection dir) => dir switch
        {
            EDirection.DirUp => "UP   ↑",
            EDirection.DirDown => "DOWN ↓",
            EDirection.DirLeft => "LEFT ←",
            EDirection.DirRight => "RIGHT→",
            _ => dir.ToString()
        };

        public static string PosToStr(Vector2Info pos)
            => pos is null ? "(?,?)" : $"({pos.X},{pos.Y})";
    }
    public class ServerPacketHandler 
    {
        internal static void HANDLE_Invalid(PacketSession session, IMessage message)
        {
            throw new NotImplementedException();
        }
        internal static void HANDLE_S_JwtLoginReply(PacketSession session, S_JwtLoginReply reply)
        {
            switch (reply.Result) // 프로토 C# 코드 생성 시 보통 PascalCase enum이 됩니다 (Success 등). 필요하면 이름 맞춰 수정
            {
                case ELoginResult.Success: // 또는 LoginResult.Success

                    //Console.WriteLine($"[JWT VALIDATION OK]");
                   
                    UnityEngine.Debug.Log($"[LOGIN OK] accountId={reply.Result}");
                    AuthNotice_UI.Instance.gameObject.SetActive(true);
                    AuthNotice_UI.Instance.ShowNotice(NoticeCode.LoginSuccess);
                    CharacterList_UI.Instance.gameObject.SetActive(true);
                    break;

                case ELoginResult.InvalidToken: // InvalidToken

                    //Console.WriteLine("[JWT VALIDATION] Invalid token. Please re-auth.");
                    UnityEngine.Debug.Log("[JWT VALIDATION] Invalid token. Please re-auth.");
                    //토큰 재발급 UX로 전환
                    break;

                case ELoginResult.TokenExpired: // TokenExpired

                    Console.WriteLine("[JWT VALIDATION] Token expired. Get a new token.");
                    UnityEngine.Debug.Log("[JWT VALIDATION] Token expired. Get a new token.");
                    // 리프레시 토큰/재로그인 유도
                    break;

                case ELoginResult.ServerError: // ServerError
                default:

                    //Console.WriteLine($"[JWT VALIDATION] Server error (code={(int)reply.Result}). Try again later.");
                    UnityEngine.Debug.Log($"[JWT VALIDATION] Server error (code={(int)reply.Result}). Try again later.");
                    break;
            }
        }
        internal static void HANDLE_S_CreateCharacterReply(PacketSession session, S_CreateCharacterReply reply)
        {
            var result = reply;
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.CreateCharacterSuccess);
            UnityEngine.Debug.Log($"[CreateCharacterReply] 결과: {result.Success}.");
            UnityEngine.Debug.Log($"[CreateCharacterReply] 결과: {result.Detail}.");
            
        }  

        internal static void HANDLE_S_CharacterListReply(PacketSession session, S_CharacterListReply reply)
        {
            UnityEngine.Debug.Log($"[S_CharacterListReply] 전송받음");
            AuthNotice_UI.Instance.gameObject.SetActive(true); 
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.RecvCharacterListSuccess);
            if (reply.Characters == null)
            {
                UnityEngine.Debug.Log("계정 내 생성된 캐릭터가 없습니다.");
            }
            CharacterList_UI.Instance.SetCharacterList(reply.Characters);
        }

        internal static void HANDLE_S_EnterGame(PacketSession session, S_EnterGame game)
        {
            AuthNotice_UI.Instance.gameObject.SetActive(false);
            Console.WriteLine("[S_EnterGame] 게임 접속 완료");
            Debug.Log("[S_EnterGame] 게임 접속 완료");
        }

        internal static void HANDLE_S_PlayerList(PacketSession session, S_PlayerList list)
        {
            Console.WriteLine("[S_PlayerList] 내가 접속해서 다른사람의 리스트 받아옴");
            Debug.Log("[S_PlayerList] 내가 접속해서 다른사람의 리스트 받아옴");
        }

        internal static void HANDLE_S_BroadcastPlayerEnter(PacketSession session, S_BroadcastPlayerEnter enter)
        {
            Debug.Log("[S_BroadcastPlayerEnter] 누군가 접속해서 그 정보를 받아옴");
        }

        internal static void HANDLE_S_BroadcastPlayerLeave(PacketSession session, S_BroadcastPlayerLeave leave)
        {
            Debug.Log("[S_BroadcastPlayerLeave] 누군가 나가서 그 정보를 받아옴");
        }

        internal static void HANDLE_S_PlayerMoveReply(PacketSession session, S_PlayerMoveReply reply)
        {
            // 안전하게 널 체크
            var pos = reply.NewPos;
            var posStr = NetDebug.PosToStr(pos);
            var dirStr = NetDebug.DirToStr(reply.Direction);

            // result/tick 필드가 없을 수도 있으니 Try 포맷
            string resultStr = reply?.Result.ToString() ?? "N/A";
            int tick = reply?.Tick ?? -1;

            string meTag = (reply.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (ME)" : "";
            Debug.Log(
                $"[S_PlayerMoveReply] pid={reply.PlayerId}{meTag} " +
                $"dir={dirStr} pos={posStr} result={resultStr} tick={tick}");
            Console.WriteLine(
                $"[S_PlayerMoveReply] pid={reply.PlayerId}{meTag} " +
                $"dir={dirStr} pos={posStr} result={resultStr} tick={tick}");
        }

        internal static void HANDLE_S_BroadcastPlayerMove(PacketSession session, S_BroadcastPlayerMove move)
        {
            //foreach (var m in move)
            //{ 
            //    PlayerMAnager.Isntacen.Move(  m.PlayerId);
            //}
            int tick = move?.Tick ?? -1;
            Console.WriteLine($"[S_BroadcastPlayerMove] tick={tick} count={move.PlayerMoves.Count}");

            foreach (var m in move.PlayerMoves)
            {
                var pos = m.NewPos;
                var posStr = NetDebug.PosToStr(pos);
                var dirStr = NetDebug.DirToStr(m.Direction);
                string meTag = (m.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (ME)" : "";

                Console.WriteLine($"  - pid={m.PlayerId}{meTag} dir={dirStr} pos={posStr}");
            }
        }

        internal static void HANDLE_S_ChangeRoomBegin(PacketSession session, S_ChangeRoomBegin begin)
        {
            Console.WriteLine($"[S_ChangeRoomBegin] Begin Change Room");

            var pkt = new Google.Protobuf.Protocol.C_ChangeRoomReady {
                TransitionId = begin.TransitionId,
            };

            session.Send(ServerPacketManager.MakeSendBuffer(pkt));
        }

        internal static void HANDLE_S_ChangeRoomCommit(PacketSession session, S_ChangeRoomCommit commit)
        {
            Console.WriteLine($"[S_ChangeRoomCommit] Room Has Change into ...");
        }

        internal static void HANDLE_S_LeaveGame(PacketSession session, S_LeaveGame game)
        {
            Console.WriteLine($"[S_LeaveGame] Game Has left.");
        }
    }
}