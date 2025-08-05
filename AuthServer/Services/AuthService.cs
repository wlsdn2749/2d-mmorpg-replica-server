using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Mmorpg2d.Auth;
using Grpc.Core;

namespace AuthServer
{
    public class AuthService : Auth.AuthBase
    { 
        
        // 회원가입: 입력 검증 -> 중복 체크 -> ... -> 성공/실패 반환
        // gRPC    : 클라이언트 -> 서버 직접 호출
        public override Task<RegisterReply> Register(RegisterRequest request, ServerCallContext context)
        {
           // 입력 검증

           // 중복 체크


           // ...


           // 결과 반환
        }
    }
}
