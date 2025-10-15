using Packet;
using Protocol;
using ServerCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DummyClientCS
{
    internal class SessionManager
    {
        /* ─── 싱글톤 ────────────────────────────────────────── */
        static SessionManager _session = new SessionManager();
        public static SessionManager Instance { get { return _session; } }

        /* ─── 상태 테이블 ────────────────────────────────────────── */
        object _lock = new object();
        List<ServerSession> _sessions = new List<ServerSession>();
        
        bool _canSendPackets = false; // TCP HandShake 송신 후에

        public void SetCanSendPackets(bool flag)
        {
            _canSendPackets = flag;
        }
       
        public ServerSession Generate()
        {
            lock (_lock)
            {
                ServerSession session = new ServerSession();
                _sessions.Add(session);
                return session;
            }
        }

        public async Task SendForEachJWTLoginAsync(string jwt)
        {
            if (!_canSendPackets) return;

            lock(_lock)
            {
                foreach(ServerSession session in _sessions)
                {
                    var accessToken = jwt;
                    var pkt = new Google.Protobuf.Protocol.C_JwtLoginRequest { AccessToken = accessToken };

                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                }
            }
        }

        public async Task SendForEachCreateCharacterAsync(string username)
        {
            if(!_canSendPackets) return;

            lock(_lock)
            {
                foreach(ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_CreateCharacterRequest
                    {
                        Username = username,
                        Gender = Google.Protobuf.Protocol.EGender.GenderMale,
                        Region = Google.Protobuf.Protocol.ERegion.RegionGo,
                    };

                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                }
            }
        }

        public async Task SendForEachGetCharacterList()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_CharacterListRequest { };

                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                }

            }
        }

        public async Task SendForEachEnterGame(int idx)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_EnterGame
                    {
                        PlayerIndex = idx
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                }
            }
        }

        public async Task SendForEachMove(int dir)
        {
            if( !_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    // 현재 플레이어 좌표 가져오기 (예: 타일 좌표 또는 월드 좌표)
                    // 이 부분은 실제 프로젝트에서 플레이어 위치를 얻는 코드로 교체하세요
                    int targetX = 1000;
                    int targetY = 1000;

                    var clickPos = new Google.Protobuf.Protocol.Vector2Info();

                    switch (dir)
                    {
                        case 0: // UP
                            clickPos.X = 0;
                            clickPos.Y = +targetY;
                            break;
                        case 1: // DOWN
                            clickPos.X = 0;
                            clickPos.Y = -targetY;
                            break;
                        case 2: // LEFT
                            clickPos.X = -targetX;
                            clickPos.Y = 0;
                            break;
                        case 3: // RIGHT
                            clickPos.X = targetX;
                            clickPos.Y = 0;
                            break;
                        default:
                            clickPos.X = 0;
                            clickPos.Y = 0;
                            break;
                    }

                    var pkt = new Google.Protobuf.Protocol.C_PlayerMoveRequest
                    {
                        ClickWorldPos = clickPos
                    };

                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                }
            }
        }
        public async Task SendForLeave()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach(ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_LeaveGame
                    {
                        Reason = Google.Protobuf.Protocol.ELeaveReason.LeaveChangeCharacter // 기본값: 캐릭터 변경
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                }
            }
        }

        // 다양한 종료 사유별 테스트 메소드들
        public async Task SendLeaveGameWithReason(Google.Protobuf.Protocol.ELeaveReason reason)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_LeaveGame
                    {
                        Reason = reason
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine($"C_LeaveGame 패킷 전송됨 - 사유: {reason}");
                }
            }
        }

        // 로그아웃 (JWT 인증 상태로 복귀)
        public async Task SendLeaveForLogout()
        {
            await SendLeaveGameWithReason(Google.Protobuf.Protocol.ELeaveReason.LeaveLogout);
        }

        // 캐릭터 변경 (캐릭터 선택창으로 복귀) 
        public async Task SendLeaveForCharacterChange()
        {
            await SendLeaveGameWithReason(Google.Protobuf.Protocol.ELeaveReason.LeaveChangeCharacter);
        }

        // 룸 이동
        public async Task SendLeaveForRoomChange()
        {
            await SendLeaveGameWithReason(Google.Protobuf.Protocol.ELeaveReason.LeaveChangeRoom);
        }

        // 연결 해제
        public async Task SendLeaveForDisconnect()
        {
            await SendLeaveGameWithReason(Google.Protobuf.Protocol.ELeaveReason.LeaveDisconnect);
        }

        public async Task SendForEachAttack()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PlayerAttackRequest
                    {
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                }
            }
        }

        // 인벤토리 조회 요청
        public async Task SendInventoryRequest()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_InventoryRequest
                    {
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine("인벤토리 조회 요청을 전송했습니다.");
                }
            }
        }

        // 퀵슬롯 포션 사용 (슬롯 30번 - 체력 회복 포션)
        public async Task SendUseQuickSlotPotion()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    // 퀵슬롯 30번 (체력 회복 포션) 사용
                    var pkt = new Google.Protobuf.Protocol.C_ItemUseRequest
                    {
                        SlotIndex = 30
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine("퀵슬롯 30번 체력 포션 사용 요청을 전송했습니다.");
                }
            }
        }

        // 지정된 슬롯의 아이템 사용
        public async Task SendItemUseRequest(int slotIndex)
        {
            if (!_canSendPackets) return;

            if (slotIndex < 0 || slotIndex > 39)
            {
                Console.WriteLine("슬롯 번호는 0~39 사이여야 합니다.");
                return;
            }

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_ItemUseRequest
                    {
                        SlotIndex = slotIndex
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine($"슬롯 {slotIndex}번 아이템 사용 요청을 전송했습니다.");
                }
            }
        }

        // 룸 채팅 전송
        public async Task SendRoomChat(string message)
        {
            if (!_canSendPackets) return;

            if (string.IsNullOrWhiteSpace(message))
            {
                Console.WriteLine("메시지가 비어있습니다.");
                return;
            }

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var chatInfo = new Google.Protobuf.Protocol.PlayerChatInfo
                    {
                        NonePlayer = true, // 클라이언트->서버 전송시
                        Message = message,
                        ChatType = Google.Protobuf.Protocol.EChatType.ChatRoom
                    };

                    var pkt = new Google.Protobuf.Protocol.C_PlayerChat
                    {
                        PlayerChatInfo = chatInfo
                    };

                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine($"[룸 채팅] '{message}' 전송했습니다.");
                }
            }
        }

        // 전체 채팅 전송
        public async Task SendAllChat(string message)
        {
            if (!_canSendPackets) return;

            if (string.IsNullOrWhiteSpace(message))
            {
                Console.WriteLine("메시지가 비어있습니다.");
                return;
            }

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var chatInfo = new Google.Protobuf.Protocol.PlayerChatInfo
                    {
                        NonePlayer = true, // 클라이언트->서버 전송시
                        Message = message,
                        ChatType = Google.Protobuf.Protocol.EChatType.ChatAll
                    };

                    var pkt = new Google.Protobuf.Protocol.C_PlayerChat
                    {
                        PlayerChatInfo = chatInfo
                    };

                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine($"[전체 채팅] '{message}' 전송했습니다.");
                }
            }
        }

        // 캐릭터 삭제 요청
        public async Task SendDeleteCharacterRequest(int characterIndex)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_DeleteCharacterRequest
                    {
                        CharacterIndex = characterIndex
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine($"캐릭터 인덱스 {characterIndex} 삭제 요청을 전송했습니다.");
                }
            }
        }

        // 테스트용 아이템 지급 요청
        public async Task SendGiveItemRequest(int itemId, int count)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_GiveItemRequest
                    {
                        ItemId = itemId,
                        Count = count
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine($"아이템 지급 요청 전송: ItemID={itemId}, Count={count}");
                }
            }
        }

        // 플레이어 리스폰 준비 완료 신호 전송
        public async Task SendPlayerDeathReady()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PlayerDeathReady
                    {
                        // 빈 패킷 - 단순히 리스폰 준비 완료를 알림
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.WriteLine("리스폰 준비 완료 신호를 서버에 전송했습니다.");
                }
            }
        }

        // ========== 파티 시스템 테스트 ==========

        // 파티 초대 요청
        public async Task SendPartyInviteRequest(int targetPid)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyInviteRequest
                    {
                        TargetPid = targetPid
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine($"[파티 초대] PlayerId {targetPid}에게 파티 초대를 보냈습니다.");
                    Console.ResetColor();
                }
            }
        }

        // 파티 초대 응답 (수락/거절)
        public async Task SendPartyInviteResponse(int partyId, bool accept)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyInviteResponse
                    {
                        PartyId = partyId,
                        Accept = accept
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = accept ? ConsoleColor.Green : ConsoleColor.Red;
                    Console.WriteLine($"{(accept ? "[파티 수락]" : "[파티 거절]")} 파티ID {partyId}에 대한 응답을 보냈습니다.");
                    Console.ResetColor();
                }
            }
        }

        // 파티 탈퇴
        public async Task SendPartyLeave()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyLeave
                    {
                        SelfLeave = true
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Yellow;
                    Console.WriteLine("[파티 탈퇴] 파티를 나갑니다.");
                    Console.ResetColor();
                }
            }
        }

        // 파티원 강퇴
        public async Task SendPartyKick(int targetPid)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyLeave
                    {
                        TargetPid = targetPid
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine($"[파티 강퇴] PlayerId {targetPid}를 파티에서 강퇴했습니다.");
                    Console.ResetColor();
                }
            }
        }

        // ========== 공개 파티 시스템 (56-63번 프로토콜) ==========

        // 파티 생성 (파티명 지정)
        public async Task SendPartyCreateRequest(string partyName)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyCreateRequest
                    {
                        PartyName = partyName
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine($"[파티 생성] 파티명 '{partyName}'으로 파티 생성 요청을 보냈습니다.");
                    Console.ResetColor();
                }
            }
        }

        // 전체 파티 리스트 조회
        public async Task SendPartyListRequest()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyList
                    {
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine("[파티 리스트] 전체 파티 목록 조회 요청을 보냈습니다.");
                    Console.ResetColor();
                }
            }
        }

        // 파티 가입 요청
        public async Task SendPartyJoinRequest(int partyId)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyJoinRequest
                    {
                        PartyId = partyId
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine($"[파티 가입 요청] 파티ID {partyId}에 가입 요청을 보냈습니다.");
                    Console.ResetColor();
                }
            }
        }

        // 파티 가입 요청 응답 (리더 전용 - 수락/거절)
        public async Task SendPartyJoinResponse(int partyId, bool accept)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyJoinResponse
                    {
                        PartyId = partyId,
                        Accept = accept
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = accept ? ConsoleColor.Green : ConsoleColor.Red;
                    Console.WriteLine($"[리더 응답] 파티ID {partyId}의 가입 요청을 {(accept ? "수락" : "거절")}했습니다.");
                    Console.ResetColor();
                }
            }
        }

        // 파티 가입 요청 리스트 조회 (리더 전용)
        public async Task SendPartyJoinRequestListQuery(int partyId)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyJoinRequestList
                    {
                        PartyId = partyId
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine($"[요청 리스트 조회] 파티ID {partyId}의 가입 요청 목록을 조회합니다.");
                    Console.ResetColor();
                }
            }
        }

        // 파티 가입 요청 응답 (리더 전용 - requesterPid 지정)
        public async Task SendPartyJoinResponseWithPid(int partyId, int requesterPid, bool accept)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_PartyJoinResponse
                    {
                        PartyId = partyId,
                        RequesterPid = requesterPid,
                        Accept = accept
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = accept ? ConsoleColor.Green : ConsoleColor.Red;
                    Console.WriteLine($"[리더 응답] PlayerId {requesterPid}의 가입 요청을 {(accept ? "수락" : "거절")}했습니다.");
                    Console.ResetColor();
                }
            }
        }

        // ========== 장비 시스템 테스트 ==========

        // 장비 장착 요청
        public async Task SendEquipItemRequest(int inventorySlotIndex)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_EquipItemRequest
                    {
                        InventorySlotIndex = inventorySlotIndex
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine($"[장비 장착 요청] 인벤토리 슬롯 {inventorySlotIndex}의 아이템 장착 요청을 보냈습니다.");
                    Console.ResetColor();
                }
            }
        }

        // 장비 해제 요청
        public async Task SendUnequipItemRequest(int slotTypeValue)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_UnequipItemRequest
                    {
                        SlotType = (Google.Protobuf.Protocol.EEquipmentSlotType)slotTypeValue
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine($"[장비 해제 요청] 슬롯 타입 {slotTypeValue}의 장비 해제 요청을 보냈습니다.");
                    Console.ResetColor();
                }
            }
        }

        // 장비 정보 조회 요청
        public async Task SendEquipmentInfoRequest()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_EquipmentInfoRequest
                    {
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine("[장비 정보 조회] 현재 장착 중인 장비 정보를 요청했습니다.");
                    Console.ResetColor();
                }
            }
        }

        // NPC 상호작용 요청
        public async Task SendNpcInteractRequest(int interactionType)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_NpcInteractRequest
                    {
                        InteractionType = interactionType
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Green;
                    string interactionName = interactionType switch
                    {
                        0 => "Talk",
                        1 => "Shop",
                        2 => "Quest",
                        _ => "Unknown"
                    };
                    Console.WriteLine($"[NPC Interact] Interaction type: {interactionName} ({interactionType})");
                    Console.ResetColor();
                }
            }
        }

        // NPC 상점 구매 요청
        public async Task SendNpcShopBuyRequest(int npcId, int itemId, int quantity)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_NpcShopBuyRequest
                    {
                        NpcId = npcId,
                        ItemId = itemId,
                        Quantity = quantity
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.WriteLine($"[NPC Shop Buy] NpcId: {npcId}, ItemId: {itemId}, Quantity: {quantity}");
                    Console.ResetColor();
                }
            }
        }

        // NPC 상점 판매 요청
        public async Task SendNpcShopSellRequest(int npcId, int itemId, int quantity)
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_NpcShopSellRequest
                    {
                        NpcId = npcId,
                        ItemId = itemId,
                        Quantity = quantity
                    };
                    session.Send(ServerPacketManager.MakeSendBuffer(pkt));
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.WriteLine($"[NPC Shop Sell] NpcId: {npcId}, ItemId: {itemId}, Quantity: {quantity}");
                    Console.ResetColor();
                }
            }
        }
    }
}
