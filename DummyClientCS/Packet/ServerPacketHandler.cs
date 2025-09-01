using DummyClientCS;
using Google.Protobuf;
using Google.Protobuf.Protocol;
using Microsoft.VisualBasic;
using ServerCore;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

        public static string MoveResultToStr(EMoveResult moveResult) => moveResult switch
        {
            EMoveResult.MoveUnknown => "Unknown",
            EMoveResult.MoveOk => "Ok",
            EMoveResult.MoveDir => "Dir",
            EMoveResult.MoveBlocked => "Blocked",
            EMoveResult.MoveCooldown => "Cooldown",
            _ => moveResult.ToString()

        };
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

                    Console.WriteLine($"[JWT VALIDATION OK]");
                    // Unity라면: Debug.Log($"[LOGIN OK] accountId={reply.AccountId}");

                    // 다음 단계로 진행:
                    // - 캐릭터 리스트 요청
                    // - 바로 게임 입장 패킷 보내기 등
                    // Send_C_CHARACTER_LIST_REQUEST(session);
                    break;

                case ELoginResult.InvalidToken: // InvalidToken

                    Console.WriteLine("[JWT VALIDATION] Invalid token. Please re-auth.");
                    // 토큰 재발급 UX로 전환
                    break;

                case ELoginResult.TokenExpired: // TokenExpired

                    Console.WriteLine("[JWT VALIDATION] Token expired. Get a new token.");
                    // 리프레시 토큰/재로그인 유도
                    break;

                case ELoginResult.ServerError: // ServerError
                default:

                    Console.WriteLine($"[JWT VALIDATION] Server error (code={(int)reply.Result}). Try again later.");
                    break;
            }
        }
        internal static void HANDLE_S_CreateCharacterReply(PacketSession session, S_CreateCharacterReply reply)
        {
            var result = reply;
            Console.WriteLine($"[CreateCharacterReply] 결과: {result.Success}.");
            Console.WriteLine($"[CreateCharacterReply] 결과: {result.Detail}.");
        }

        internal static void HANDLE_S_CharacterListReply(PacketSession session, S_CharacterListReply reply)
        {
            foreach(var character in reply.Characters)
            {
                Console.WriteLine(character);
            }
        }

        internal static void HANDLE_S_EnterGame(PacketSession session, S_EnterGame game)
        {
            Console.WriteLine("[S_EnterGame] 게임 접속 완료");
        }

        internal static void HANDLE_S_PlayerList(PacketSession session, S_PlayerList list)
        {
            Console.WriteLine("[S_PlayerList] 내가 접속해서 다른사람의 리스트 받아옴");
        }

        internal static void HANDLE_S_BroadcastPlayerEnter(PacketSession session, S_BroadcastPlayerEnter enter)
        {
            Console.WriteLine("[S_BroadcastPlayerEnter] 누군가 접속해서 그 정보를 받아옴");
        }

        internal static void HANDLE_S_BroadcastPlayerLeave(PacketSession session, S_BroadcastPlayerLeave leave)
        {
            Console.WriteLine("[S_BroadcastPlayerLeave] 누군가 나가서 그 정보를 받아옴");
        }

        internal static void HANDLE_S_PlayerMoveReply(PacketSession session, S_PlayerMoveReply reply)
        {
            // 안전하게 널 체크
            var pos = reply.NewPos;
            var posStr = NetDebug.PosToStr(pos);
            var dirStr = NetDebug.DirToStr(reply.Direction);
            var mResult = NetDebug.MoveResultToStr(reply.Result);
            

            // result/tick 필드가 없을 수도 있으니 Try 포맷
            string resultStr = reply?.Result.ToString() ?? "N/A";
            int tick = reply?.Tick ?? -1;

            string meTag = (reply.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (ME)" : "";

            Console.WriteLine(
                $"[S_PlayerMoveReply] pid={reply.PlayerId}{meTag} " +
                $"dir={dirStr} pos={posStr} result={resultStr} tick={tick} mResult={mResult}" );
        }

        internal static void HANDLE_S_BroadcastPlayerMove(PacketSession session, S_BroadcastPlayerMove move)
        {
            int tick = move?.Tick ?? -1;
            Console.WriteLine($"[S_BroadcastPlayerMove] tick={tick} count={move.PlayerMoves.Count}");

            foreach (var m in move.PlayerMoves)
            {
                var pos = m.NewPos;
                var posStr = NetDebug.PosToStr(pos);
                var dirStr = NetDebug.DirToStr(m.Direction);
                var mResult = NetDebug.MoveResultToStr(m.Result);
                string meTag = (m.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (ME)" : "";

                Console.WriteLine($"  - pid={m.PlayerId}{meTag} dir={dirStr} pos={posStr} mResult={mResult}");
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

        internal static void HANDLE_S_BroadcastMonsterDeath(PacketSession session, S_BroadcastMonsterDeath death)
        {
            Console.WriteLine($"[S_BroadcastMonsterDeath] Monster Has been dead.");
        }

        internal static void HANDLE_S_SpawnMonster(PacketSession session, S_SpawnMonster monster)
        {
            Console.WriteLine($"[S_SpawnMonster] Monster Has been spawned");
        }

        internal static void HANDLE_S_DespawnMonster(PacketSession session, S_DespawnMonster monster)
        {
            Console.WriteLine($"[S_DespawnMonster] Monster Has been despawned");
        }

        internal static void HANDLE_S_BroadcastMonsterMove(PacketSession session, S_BroadcastMonsterMove move)
        {
            Console.WriteLine($"[S_BroadcastMonsterMove] Monster Has been moved");
        }

        internal static void HANDLE_S_BroadcastMonsterAttack(PacketSession session, S_BroadcastMonsterAttack attack)
        {
            Console.WriteLine($"[S_BroadcastMonsterAttack] Monster Has been Attacked");
        }

        internal static void HANDLE_S_BroadcastPlayerAttack(PacketSession session, S_BroadcastPlayerAttack attack)
        {
            Console.WriteLine($"[S_BroadcastPlayerAttack] Other Player Attacked ");
        }

        // 인벤토리 조회 응답 처리
        internal static void HANDLE_S_InventoryReply(PacketSession session, S_InventoryReply reply)
        {
            Console.WriteLine("=== 인벤토리 정보 ===");
            Console.WriteLine($"총 {reply.Slots.Count}개의 아이템이 있습니다.");
            
            if (reply.Slots.Count == 0)
            {
                Console.WriteLine("인벤토리가 비어있습니다.");
                return;
            }

            // 일반 슬롯 (0~29)
            Console.WriteLine("\n일반 슬롯:");
            var normalSlots = reply.Slots.Where(s => !s.IsQuickslot).OrderBy(s => s.SlotIndex);
            foreach (var slot in normalSlots)
            {
                Console.WriteLine($"  슬롯[{slot.SlotIndex:D2}] 아이템ID:{slot.ItemId} 수량:{slot.Count}");
            }

            // 퀵 슬롯 (30~39)  
            Console.WriteLine("\n퀵 슬롯:");
            var quickSlots = reply.Slots.Where(s => s.IsQuickslot).OrderBy(s => s.SlotIndex);
            foreach (var slot in quickSlots)
            {
                Console.WriteLine($"  퀵슬롯[{slot.SlotIndex:D2}] 아이템ID:{slot.ItemId} 수량:{slot.Count}");
            }
            Console.WriteLine("========================");
        }

        // 아이템 사용 응답 처리
        internal static void HANDLE_S_ItemUseReply(PacketSession session, S_ItemUseReply reply)
        {
            if (reply.Success)
            {
                Console.WriteLine("아이템 사용 성공!");
                Console.WriteLine("HP가 회복되었습니다!");
            }
            else
            {
                Console.WriteLine("아이템 사용 실패!");
                Console.WriteLine($"오류: {reply.ErrorMessage}");
            }
        }

        // 인벤토리 업데이트 브로드캐스트 처리
        internal static void HANDLE_S_InventoryUpdate(PacketSession session, S_InventoryUpdate update)
        {
            Console.WriteLine("인벤토리가 업데이트되었습니다:");
            foreach (var slot in update.ChangedSlots)
            {
                if (slot.Count > 0)
                {
                    string slotType = slot.IsQuickslot ? "퀵슬롯" : "일반슬롯";
                    Console.WriteLine($"  {slotType}[{slot.SlotIndex}] 아이템ID:{slot.ItemId} 수량:{slot.Count}");
                }
                else
                {
                    Console.WriteLine($"  슬롯[{slot.SlotIndex}] 비어짐");
                }
            }
        }
    }
}