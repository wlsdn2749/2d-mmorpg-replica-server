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

        public async Task SendForEachEnterGame()
        {
            if (!_canSendPackets) return;

            lock (_lock)
            {
                foreach (ServerSession session in _sessions)
                {
                    var pkt = new Google.Protobuf.Protocol.C_EnterGame
                    {
                        PlayerIndex = 0
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
                    int playerX = 10;
                    int playerY = 10;

                    var clickPos = new Google.Protobuf.Protocol.Vector2Info();

                    switch (dir)
                    {
                        case 0: // UP
                            clickPos.X = playerX;
                            clickPos.Y = playerY - 1;
                            break;
                        case 1: // DOWN
                            clickPos.X = playerX;
                            clickPos.Y = playerY + 1;
                            break;
                        case 2: // LEFT
                            clickPos.X = playerX - 1;
                            clickPos.Y = playerY;
                            break;
                        case 3: // RIGHT
                            clickPos.X = playerX + 1;
                            clickPos.Y = playerY;
                            break;
                        default:
                            clickPos.X = playerX;
                            clickPos.Y = playerY;
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
    }
}
