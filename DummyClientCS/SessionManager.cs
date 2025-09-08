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
                        Gender = 0,
                        Region = 0
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
                    Console.WriteLine($"📤 C_LeaveGame 패킷 전송됨 - 사유: {reason}");
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
                Console.WriteLine("❌ 슬롯 번호는 0~39 사이여야 합니다.");
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
    }
}
