using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Mmorpg2d.Auth;
using BCrypt.Net;
using Grpc.Core;
using AuthServer.Contracts.Repository;
using AuthServer;

namespace AuthServer
{
    public class AuthService : Auth.AuthBase
    {
        private readonly IUserRepository _userRepository;
        private readonly IJwtTokenGenerator _jwt;
        public AuthService(IUserRepository userRepository, IJwtTokenGenerator jwt)
        {
            _userRepository = userRepository;
            _jwt = jwt;
        }
        // 회원가입: 입력 검증 -> 중복 체크 -> ... -> 성공/실패 반환
        // gRPC    : 클라이언트 -> 서버 직접 호출
        public override async Task<RegisterReply> Register(RegisterRequest request, ServerCallContext context)
        {

            var v = await RegisterValidator.ValidateAsync(request, _userRepository);
            if (!v.IsValid)
                return new() { Success = false, Detail = v.Detail };

            var passwordHash = BCrypt.Net.BCrypt.HashPassword(request.Password); // 해쉬
            await _userRepository.CreateAsync(request.Email, passwordHash); // 생성

            return new() { Success = true, Detail = "가입 완료" };
        }


        // 로그인  : 입력 검증 -> 성공/실패 + jwt 토큰 반환
        // gRPC    : 클라이언트 -> 서버 직접 호출
        public override async Task<LoginReply> Login(LoginRequest request, ServerCallContext context)
        {
            // 공백, Email 존재 여부 검증
            var v = await LoginValidator.ValidateAsync(request, _userRepository);
            if (!v.IsValid)
                return new() { Success = false, Detail = v.Detail, Jwt = "" };

            // 패스워드 검증
            var user = await _userRepository.FindAsync(request.Email);
            if (user == null || !BCrypt.Net.BCrypt.Verify(request.Password, user.PasswordHash))
                return new() { Success = false, Detail = "아이디/비밀번호 불일치", Jwt = ""};

            // jwt 생성
            var token = _jwt.Generate(user.Id, user.Email);

            return new() { Success = true, Detail = "로그인 완료", Jwt = token };
        }

        // 회원가입 시, 중복 이메일 여부 요청
        public override async Task<CheckEmailReply> CheckEmail(CheckEmailRequest request, ServerCallContext context)
        {
            var exists = await _userRepository.ExistAsync(request.Email);
            
            return exists
                ? new () { Available = false, Detail = "Email already exists" }
                : new () { Available = true,  Detail = "Available" };
        }
    }
}
