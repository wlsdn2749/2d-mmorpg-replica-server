using DummyClientCS;
using Google.Protobuf;
using Google.Protobuf.Protocol;
using Microsoft.VisualBasic;
using ServerCore;
using System;
using System.Collections;
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
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"[S_NpcShopOpen] NPC Shop Open - NpcId: {open.NpcId}, ShopId: {open.ShopId}");
            Console.WriteLine("Available Items:");
            foreach (var item in open.Items)
            {
                Console.WriteLine($"  - ItemId: {item.ItemId}, Quantity: {item.Quantity}, Price: {item.Price}");
            }
            Console.ResetColor();
        }

        internal static void HANDLE_S_NpcShopBuyReply(PacketSession session, S_NpcShopBuyReply reply)
        {
            if (reply.Success)
            {
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"[S_NpcShopBuyReply] Purchase Success!");
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"[S_NpcShopBuyReply] Purchase Failed: {reply.Detail}");
            }
            Console.ResetColor();
        }

        internal static void HANDLE_S_NpcInteractReply(PacketSession session, S_NpcInteractReply reply)
        {
            string interactionType = reply.InteractionType switch
            {
                0 => "Talk",
                1 => "Shop",
                2 => "Quest",
                _ => "Unknown"
            };

            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine($"[S_NpcInteractReply] NPC Interaction - NpcId: {reply.NpcId}, Name: {reply.NpcName}, Type: {interactionType}");
            if (reply.Dialogs.Count > 0)
            {
                Console.WriteLine("Dialogs:");
                foreach (var dialog in reply.Dialogs)
                {
                    Console.WriteLine($"  - {dialog}");
                }
            }
            Console.ResetColor();
        }

        internal static void HANDLE_S_PlayerStat(PacketSession session, S_PlayerStat stat)
        {
            var info = stat.StatInfo;
            Console.WriteLine($"MaxHp : {info.MaxHp}, Hp: {info.Hp}, MaxExp: {info.MaxExp}, Exp: {info.CurExp}, Lv, {info.Level}, Money: {info.Money} ");
        }

        internal static void HANDLE_S_BroadcastPlayerTryAttack(PacketSession session, S_BroadcastPlayerTryAttack attack)
        {
            Console.WriteLine($"공격시도 전달받음 Pid : {attack.PlayerId}");
        }

        internal static void HANDLE_S_BroadcastPlayerHpChanged(PacketSession session, S_BroadcastPlayerHpChanged changed)
        {
            Console.WriteLine($"플레이어 HP 변경");
        }

        internal static void HANDLE_S_BroadcastPlayerDeath(PacketSession session, S_BroadcastPlayerDeath death)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("========================================");
            Console.WriteLine("[S_BroadcastPlayerDeath] 플레이어 사망!");
            Console.WriteLine("========================================");

            string playerInfo = death.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0 ?
                $"플레이어 {death.PlayerId} (나)" : $"플레이어 {death.PlayerId}";

            Console.WriteLine($"사망한 플레이어: {playerInfo}");
            Console.WriteLine($"킬러 몬스터 ID: {death.KillerMonsterId}");
            Console.WriteLine($"리스폰 목적지 맵: {GetSceneNameByMapId(death.MapId)} (MapID: {death.MapId})");

            // 내가 죽었다면 리스폰 준비 신호를 보낼 수 있음을 알림
            if (death.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0)
            {
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine("당신이 사망했습니다! 'p' 키를 눌러 리스폰하세요.");
            }

            Console.WriteLine("========================================");
            Console.ResetColor();
        }

        internal static void HANDLE_S_BroadcastPlayerChat(PacketSession session, S_BroadcastPlayerChat chat)
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"\n===== 채팅 메시지 수신 =====");

            foreach (var chatInfo in chat.PlayerChatInfos)
            {
                string chatTypeStr = chatInfo.ChatType == EChatType.ChatRoom ? "[룸]" : "[전체]";
                string playerIdStr = chatInfo.PlayerId > 0 ? $"플레이어{chatInfo.PlayerId}" : "알수없음";

                Console.WriteLine($"{chatTypeStr} {playerIdStr}: {chatInfo.Message}");
            }

            Console.WriteLine($"============================");
            Console.ResetColor();
        }

        internal static void HANDLE_S_GiveItemReply(PacketSession session, S_GiveItemReply reply)
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine("[S_GiveItemReply] 아이템 지급 결과");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");

            if (reply.Success)
            {
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("아이템 지급 성공!");

                if (reply.AddedSlot != null)
                {
                    Console.WriteLine($"추가된 슬롯 정보:");
                    Console.WriteLine($"   슬롯 인덱스: {reply.AddedSlot.SlotIndex}");
                    Console.WriteLine($"   아이템 ID: {reply.AddedSlot.ItemId}");
                    Console.WriteLine($"   수량: {reply.AddedSlot.Count}");
                    Console.WriteLine($"   퀵슬롯 여부: {(reply.AddedSlot.IsQuickslot ? "예" : "아니오")}");
                }

                Console.WriteLine("인벤토리 조회('i' 키)로 전체 인벤토리를 확인해보세요!");
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("아이템 지급 실패!");
                if (!string.IsNullOrEmpty(reply.ErrorMessage))
                {
                    Console.WriteLine($"오류 메시지: {reply.ErrorMessage}");
                }
            }

            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_PlayerDeathCommit(PacketSession session, S_PlayerDeathCommit commit)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("========================================");
            Console.WriteLine("[S_PlayerDeathCommit] 리스폰 승인!");
            Console.WriteLine("========================================");

            string destinationMap = GetSceneNameByMapId(commit.MapId);
            Console.WriteLine($"리스폰 목적지: {destinationMap} (MapID: {commit.MapId})");
            Console.WriteLine($"리스폰 처리가 시작됩니다...");
            Console.WriteLine($"룸 이동이 곧 완료됩니다.");

            Console.WriteLine("========================================");
            Console.ResetColor();
        }

        internal static void HANDLE_S_PartyInviteNotify(PacketSession session, S_PartyInviteNotify notify)
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine("[S_PartyInviteNotify] 파티 초대 알림!");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine($"초대자: {notify.InviterName} (PlayerId: {notify.InviterPid})");
            Console.WriteLine($"파티 ID: {notify.PartyId}");
            Console.WriteLine($"");
            Console.WriteLine($"파티에 참가하시겠습니까?");
            Console.WriteLine($"   수락하려면 's' 키를 누르고 파티 ID {notify.PartyId} 입력");
            Console.WriteLine($"   거절하려면 'f' 키를 누르고 파티 ID {notify.PartyId} 입력");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_PartyInviteReply(PacketSession session, S_PartyInviteReply reply)
        {
            Console.ForegroundColor = reply.Success ? ConsoleColor.Green : ConsoleColor.Red;
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine($"{(reply.Success ? "[성공]" : "[실패]")} [S_PartyInviteReply] 파티 초대 요청 결과");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");

            if (reply.Success)
            {
                Console.WriteLine("파티 초대를 성공적으로 보냈습니다!");
                Console.WriteLine("상대방의 응답을 기다리는 중...");
            }
            else
            {
                Console.WriteLine("파티 초대 실패!");
                if (!string.IsNullOrEmpty(reply.ErrorMessage))
                {
                    Console.WriteLine($"오류: {reply.ErrorMessage}");
                }
            }

            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_BroadcastPartyUpdate(PacketSession session, S_BroadcastPartyUpdate update)
        {
            string updateTypeStr = update.UpdateType switch
            {
                EPartyUpdateType.PartyUpdateMemberJoin => "멤버 가입",
                EPartyUpdateType.PartyUpdateMemberLeave => "멤버 탈퇴",
                EPartyUpdateType.PartyUpdateStatus => "상태 업데이트",
                EPartyUpdateType.PartyUpdateDisbanded => "파티 해체",
                _ => "알 수 없음"
            };

            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"[PartyUpdate] {updateTypeStr} | 파티원: {update.Members.Count}명");
            foreach (var member in update.Members)
            {
                Console.WriteLine($"[Id:] {member.PlayerId} | Name: {member.PlayerName} | 리더: {member.IsLeader}");
                Console.WriteLine($"[hp:] {member.Hp}/{member.MaxHp} | Lv: {member.Level}");

            }


            Console.ResetColor();
        }

        internal static void HANDLE_S_PartyCreateReply(PacketSession session, S_PartyCreateReply reply)
        {
            Console.ForegroundColor = reply.Success ? ConsoleColor.Green : ConsoleColor.Red;
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine($"[S_PartyCreateReply] {(reply.Success ? "파티 생성 성공!" : "파티 생성 실패!")}");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");

            if (reply.Success)
            {
                Console.WriteLine("파티가 성공적으로 생성되었습니다!");
                Console.WriteLine("다른 플레이어들이 파티 리스트에서 확인할 수 있습니다.");
            }
            else
            {
                Console.WriteLine($"오류: {reply.Message}");
            }

            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_PartyJoinReply(PacketSession session, S_PartyJoinReply reply)
        {
            Console.ForegroundColor = reply.Success ? ConsoleColor.Green : ConsoleColor.Red;
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine($"[S_PartyJoinReply] {(reply.Success ? "파티 가입 요청 성공!" : "파티 가입 요청 실패!")}");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");

            if (reply.Success)
            {
                Console.WriteLine("파티에 성공적으로 가입요청을 보넀습니다. ");
                Console.WriteLine("파티장이 승인하면 가입 됩니다.");
            }
            else
            {
                Console.WriteLine($"메시지: {reply.Message}");
            }

            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_PartyList(PacketSession session, S_PartyList list)
        {
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine("\n════════════════════════════════════════════════════════════════════════");
            Console.WriteLine("[S_PartyList] 전체 파티 목록");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");

            if (list.PartyInfos.Count == 0)
            {
                Console.WriteLine("현재 생성된 파티가 없습니다.");
            }
            else
            {
                Console.WriteLine($"총 {list.PartyInfos.Count}개의 파티가 있습니다.\n");

                foreach (var party in list.PartyInfos)
                {
                    Console.WriteLine($"────────────────────────────────────────");
                    Console.WriteLine($"파티 ID: {party.PartyId}");
                    Console.WriteLine($"파티명: {party.PartyName}");
                    Console.WriteLine($"인원: {party.CurMemberCount}/{party.MaxMemberCount}");
                    Console.WriteLine($"리더 ID: {party.PartyLeaderId}");

                    if (party.Members.Count > 0)
                    {
                        Console.WriteLine($"멤버 목록:");
                        foreach (var member in party.Members)
                        {
                            string leaderTag = member.IsLeader ? " [리더]" : "";
                            string meTag = (member.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (나)" : "";
                            Console.WriteLine($"  - PlayerId: {member.PlayerId}{meTag}{leaderTag} | Lv.{member.Level} | HP:{member.Hp}/{member.MaxHp}");
                        }
                    }
                }
            }

            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_PartyJoinNotify(PacketSession session, S_PartyJoinNotify notify)
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine("[S_PartyJoinNotify] 파티 가입 요청 알림 (리더 전용)");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine($"요청자 PlayerId: {notify.JoinPlayerId}");
            Console.WriteLine($"파티 ID: {notify.PartyId}");
            Console.WriteLine($"리더 ID: {notify.LeaderId}");
            Console.WriteLine("");
            Console.WriteLine($"가입 요청을 수락/거절하려면:");
            Console.WriteLine($"  Program.cs에서 공개 파티 응답 기능을 사용하세요");
            Console.WriteLine($"  (메뉴에서 리더 응답 기능 추가 필요)");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_PartyJoinRequestList(PacketSession session, S_PartyJoinRequestList list)
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"\n[S_PartyJoinRequestList] 파티ID {list.PartyId} 가입 요청: {list.Requesters.Count}건");

            if (list.Requesters.Count == 0)
            {
                Console.WriteLine("  → 대기 중인 요청이 없습니다.");
            }
            else
            {
                foreach (var requester in list.Requesters)
                {
                    Console.WriteLine($"  → [{requester.PlayerId}] {requester.PlayerName} (Lv.{requester.Level})");
                }
            }

            Console.ResetColor();
        }

        internal static void HANDLE_S_EquipItemReply(PacketSession session, S_EquipItemReply reply)
        {
            Console.ForegroundColor = reply.Success ? ConsoleColor.Green : ConsoleColor.Red;
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine($"[S_EquipItemReply] {(reply.Success ? "장비 장착 성공!" : "장비 장착 실패!")}");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");

            if (reply.Success)
            {
                string slotTypeName = reply.SlotType switch
                {
                    EEquipmentSlotType.EquipmentWeapon => "무기",
                    EEquipmentSlotType.EquipmentHelmet => "투구",
                    EEquipmentSlotType.EquipmentArmor => "갑옷",
                    EEquipmentSlotType.EquipmentPants => "바지",
                    _ => "알 수 없음"
                };

                Console.WriteLine($"장착 부위: {slotTypeName} (슬롯 타입: {(int)reply.SlotType})");

                if (reply.ChangedSlotInfo != null)
                {
                    Console.WriteLine($"\n인벤토리 변경 사항:");
                    Console.WriteLine($"  슬롯 인덱스: {reply.ChangedSlotInfo.SlotIndex}");
                    Console.WriteLine($"  아이템 ID: {reply.ChangedSlotInfo.ItemId}");
                    Console.WriteLine($"  수량: {reply.ChangedSlotInfo.Count}");
                }

                if (reply.ReturnedEquipmentSlot != null && reply.ReturnedEquipmentSlot.ItemId > 0)
                {
                    Console.WriteLine($"\n기존 장비가 인벤토리로 복귀:");
                    Console.WriteLine($"  슬롯 인덱스: {reply.ReturnedEquipmentSlot.SlotIndex}");
                    Console.WriteLine($"  아이템 ID: {reply.ReturnedEquipmentSlot.ItemId}");
                }

                Console.WriteLine($"\n장비 정보 조회('e' 키)로 전체 장비를 확인해보세요!");
            }
            else
            {
                Console.WriteLine($"오류: {reply.ErrorMessage}");
            }

            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_UnequipItemReply(PacketSession session, S_UnequipItemReply reply)
        {
            Console.ForegroundColor = reply.Success ? ConsoleColor.Green : ConsoleColor.Red;
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.WriteLine($"[S_UnequipItemReply] {(reply.Success ? "장비 해제 성공!" : "장비 해제 실패!")}");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");

            if (reply.Success)
            {
                string slotTypeName = reply.SlotType switch
                {
                    EEquipmentSlotType.EquipmentWeapon => "무기",
                    EEquipmentSlotType.EquipmentHelmet => "투구",
                    EEquipmentSlotType.EquipmentArmor => "갑옷",
                    EEquipmentSlotType.EquipmentPants => "바지",
                    _ => "알 수 없음"
                };

                Console.WriteLine($"해제된 부위: {slotTypeName} (슬롯 타입: {(int)reply.SlotType})");

                if (reply.ReturnedEquipmentSlot != null)
                {
                    Console.WriteLine($"\n장비가 인벤토리로 복귀:");
                    Console.WriteLine($"  슬롯 인덱스: {reply.ReturnedEquipmentSlot.SlotIndex}");
                    Console.WriteLine($"  아이템 ID: {reply.ReturnedEquipmentSlot.ItemId}");
                    Console.WriteLine($"  수량: {reply.ReturnedEquipmentSlot.Count}");
                }

                Console.WriteLine($"\n인벤토리 조회('i' 키)로 인벤토리를 확인해보세요!");
            }
            else
            {
                Console.WriteLine($"오류: {reply.ErrorMessage}");
            }

            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_EquipmentInfoReply(PacketSession session, S_EquipmentInfoReply reply)
        {
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine("\n════════════════════════════════════════════════════════════════════════");
            Console.WriteLine("[S_EquipmentInfoReply] 장비 정보");
            Console.WriteLine("════════════════════════════════════════════════════════════════════════");

            if (reply.Equipments.Count == 0)
            {
                Console.WriteLine("현재 착용 중인 장비가 없습니다.");
            }
            else
            {
                foreach (var equipment in reply.Equipments)
                {
                    string slotTypeName = equipment.SlotType switch
                    {
                        EEquipmentSlotType.EquipmentWeapon => "무기",
                        EEquipmentSlotType.EquipmentHelmet => "투구",
                        EEquipmentSlotType.EquipmentArmor => "갑옷",
                        EEquipmentSlotType.EquipmentPants => "바지",
                        _ => "알 수 없음"
                    };

                    if (equipment.ItemId > 0)
                    {
                        Console.WriteLine($"\n{slotTypeName} 슬롯:");
                        Console.WriteLine($"  아이템 ID: {equipment.ItemId}");
                        Console.WriteLine($"  장비 인스턴스 ID: {equipment.EquipmentInstanceId}");
                        Console.WriteLine($"  강화 레벨: +{equipment.EnhancementLevel}");
                    }
                    else
                    {
                        Console.WriteLine($"\n{slotTypeName} 슬롯: [비어있음]");
                    }
                }
            }

            Console.WriteLine("════════════════════════════════════════════════════════════════════════");
            Console.ResetColor();
        }

        internal static void HANDLE_S_BroadcastPlayerEquipment(PacketSession session, S_BroadcastPlayerEquipment equipment)
        {
            string slotTypeName = equipment.SlotType switch
            {
                EEquipmentSlotType.EquipmentWeapon => "무기",
                EEquipmentSlotType.EquipmentHelmet => "투구",
                EEquipmentSlotType.EquipmentArmor => "갑옷",
                EEquipmentSlotType.EquipmentPants => "바지",
                _ => "알 수 없음"
            };

            string meTag = (equipment.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (나)" : "";
            string action = equipment.ItemId > 0 ? $"장착 (아이템 ID: {equipment.ItemId})" : "해제";

            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"[S_BroadcastPlayerEquipment] 플레이어 {equipment.PlayerId}{meTag}가 {slotTypeName}을(를) {action}");
            Console.ResetColor();
        }

        internal static void HANDLE_S_NpcList(PacketSession session, S_NpcList list)
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.WriteLine($"[S_NpcList] MapId: {list.MapId}, NPC Count: {list.Npcs.Count}");
            foreach (var npc in list.Npcs)
            {
                string npcTypeName = npc.NpcType switch
                {
                    0 => "Dialog",
                    1 => "Shop",
                    2 => "Quest",
                    _ => "Unknown"
                };
                Console.WriteLine($"  - NpcId: {npc.NpcId}, Name: {npc.NpcName}, Pos: ({npc.Pos.X}, {npc.Pos.Y}), Type: {npcTypeName}");
            }
            Console.ResetColor();
        }

        internal static void HANDLE_S_NpcShopSellReply(PacketSession session, S_NpcShopSellReply reply)
        {
            if (reply.Success)
            {
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"[S_NpcShopSellReply] Sell Success!");
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"[S_NpcShopSellReply] Sell Failed: {reply.Detail}");
            }
            Console.ResetColor();
        }

        internal static void HANDLE_S_PartyKickedNotify(PacketSession session, S_PartyKickedNotify notify)
        {
            Console.WriteLine($"[S_PartyKickedNotify] partyId {notify.PartyId}, kickedPlayerId:{notify.KickedPlayerId}, leaderId:{notify.LeaderId}");
        }
    }
}