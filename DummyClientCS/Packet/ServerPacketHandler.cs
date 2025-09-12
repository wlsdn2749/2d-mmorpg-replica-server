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
            Console.WriteLine("[S_PlayerList] 플레이어 리스트 및 맵 정보 수신");
            Console.WriteLine($"현재 맵ID: {list.MapId}");
            
            // 맵ID에 따른 씬 로딩 시뮬레이션
            string sceneName = GetSceneNameByMapId(list.MapId);
            Console.WriteLine($">>> 씬 로딩 시뮬레이션: '{sceneName}' 로딩 중...");
            Console.WriteLine($">>> 맵 배경 및 UI 초기화 완료");
            
            // 내 플레이어 ID 저장
            NetDebug.MyPlayerId = list.MyPlayerId;
            Console.WriteLine($"내 플레이어 ID: {list.MyPlayerId}");
            
            // 다른 플레이어 정보
            Console.WriteLine($"현재 룸에 있는 본인 포함 다른 플레이어 수: {list.Players.Count}");
            foreach (var player in list.Players)
            {
                var pos = NetDebug.PosToStr(player.Pos);
                var dir = NetDebug.DirToStr(player.Direction);
                Console.WriteLine($"  - 플레이어ID: {player.Id}, 이름: {player.Username}, 위치: {pos}, 방향: {dir}");
            }
        }

        private static string GetSceneNameByMapId(int mapId)
        {
            return mapId switch
            {
                1 => "고구려 마을",
                2 => "백제 마을", 
                3 => "사냥터",
                _ => $"알 수 없는 맵 (ID: {mapId})"
            };
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
            Console.WriteLine($"[S_ChangeRoomCommit] 방 이동 완료!");
            Console.WriteLine($"새로운 맵ID: {commit.MapId}");
            
            // 새로운 맵에 따른 씬 로딩 시뮬레이션
            string newSceneName = GetSceneNameByMapId(commit.MapId);
            Console.WriteLine($">>> 씬 전환: '{newSceneName}' 로딩 중...");
            Console.WriteLine($">>> 새로운 맵 환경 및 UI 초기화 완료");
            
        }

        internal static void HANDLE_S_LeaveGame(PacketSession session, S_LeaveGame game)
        {
            var success = game.Success;
            var detail = game.Detail;
            if (success)
            {
                Console.WriteLine($"[S_LeaveGame] Game Has left. detail : {detail}");
            }
            else
            {
                Console.WriteLine($"[S_LeaveGame Failed. detail : {detail}");
            }
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
            //Console.WriteLine($"[S_BroadcastMonsterMove] Monster Has been moved");
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

        internal static void HANDLE_S_SystemMessage(PacketSession session, S_SystemMessage message)
        {
            Console.WriteLine($"[SystemMessage] Type: {message.Type}, Message: {message.Message}");
        }

        internal static void HANDLE_S_DeleteCharacterReply(PacketSession session, S_DeleteCharacterReply reply)
        {
            if (reply.Success)
            {
                Console.WriteLine("✅ 캐릭터 삭제 성공!");
                Console.WriteLine("캐릭터가 성공적으로 삭제되었습니다. 캐릭터 리스트를 다시 조회하세요.");
            }
            else
            {
                Console.WriteLine("❌ 캐릭터 삭제 실패!");
                Console.WriteLine($"오류: {reply.ErrorMessage}");
            }
        }

        internal static void HANDLE_S_MonsterList(PacketSession session, S_MonsterList list)
        {
            Console.WriteLine($"MapId : {list.MapId} ");
            foreach(MonsterInfo monster in list.Monsters)
            {
                Console.WriteLine($"[S_MonsterList] Id:{monster.MonsterId} TypeId:{monster.MonsterTypeId} Pos:({monster.Pos.X}, {monster.Pos.Y}), Dir:{monster.Direction}");
            }
        }

        internal static void HANDLE_S_NpcShopOpen(PacketSession session, S_NpcShopOpen open)
        {
            throw new NotImplementedException();
        }

        internal static void HANDLE_S_NpcShopBuyReply(PacketSession session, S_NpcShopBuyReply reply)
        {
            throw new NotImplementedException();
        }

        internal static void HANDLE_S_NpcInteractReply(PacketSession session, S_NpcInteractReply reply)
        {
            throw new NotImplementedException();
        }

        internal static void HANDLE_S_PlayerStat(PacketSession session, S_PlayerStat stat)
        {
            var info = stat.StatInfo;
            Console.WriteLine($"MaxHp : {info.MaxHp}, Hp: {info.Hp}, Exp: {info.Exp}, Lv, {info.Level}, Money: {info.Money} ");
        }

        internal static void HANDLE_S_BroadcastPlayerTryAttack(PacketSession session, S_BroadcastPlayerTryAttack attack)
        {
            Console.WriteLine($"공격시도 전달받음 Pid : {attack.PlayerId}");
        }
    }
}