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
    }
}
