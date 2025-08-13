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

        internal static void HANDLE_S_BroadcastPlayerMove(PacketSession session, S_BroadcastPlayerMove move)
        {
            throw new NotImplementedException();
        }
    }
}