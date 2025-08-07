using DummyClientCS;
using Google.Protobuf;
using Microsoft.VisualBasic;
using Protocol;
using ServerCore;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Packet
{

    public class ServerPacketHandler
    {
        internal static void HANDLE_INVALID(PacketSession session, IMessage message)
        {
            throw new NotImplementedException();
        }

        internal static void HANDLE_S_CREATE_CHARACTER_REPLY(PacketSession session, S_CREATE_CHARACTER_REPLY rEPLY)
        {
            throw new NotImplementedException();
        }

        internal static void HANDLE_S_JWT_LOGIN_REPLY(PacketSession session, S_JWT_LOGIN_REPLY rEPLY)
        {
            throw new NotImplementedException();
        }
    }
}