using Mmorpg2d.Auth;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace DummyClientCS.Utils
{
    public static class AuthUtil
    {
        /* ---------- 회원가입 ---------- */
        public static async Task DoRegisterAsync(Auth.AuthClient client)
        {
            Console.Write("이메일 입력    : ");
            var email = Console.ReadLine() ?? "";
            Console.Write("비밀번호     : ");
            var password = ReadPassword();

            var reply = await client!.RegisterAsync(new RegisterRequest
            {
                Email = email,
                Password = password
            });

            Console.WriteLine($"[가입 결과] {reply.Success} / {reply.Detail}");
        }

        /* ---------- 로그인 ---------- */
        public static async Task<string> DoLoginAsync(Auth.AuthClient client)
        {
            Console.Write("아이디(이메일): ");
            var email = Console.ReadLine() ?? "";
            Console.Write("비밀번호     : ");
            var password = ReadPassword();

            var reply = await client!.LoginAsync(new LoginRequest
            {
                Email = email,
                Password = password
            });

            Console.WriteLine($"[로그인 결과] {reply.Success} / {reply.Detail}");
            Console.WriteLine($"JWT: {reply.Jwt}");

            return reply.Jwt;
        }

        /* ---------- 비밀번호 입력 시 에코 숨김 ---------- */
        private static string ReadPassword()
        {
            var pw = string.Empty;
            ConsoleKeyInfo key;
            do
            {
                key = Console.ReadKey(intercept: true);
                if (key.Key == ConsoleKey.Backspace && pw.Length > 0)
                {
                    pw = pw[..^1];
                    Console.Write("\b \b");
                }
                else if (!char.IsControl(key.KeyChar))
                {
                    pw += key.KeyChar;
                    Console.Write('*');
                }
            } while (key.Key != ConsoleKey.Enter);
            Console.WriteLine();
            return pw;
        }
    }
}
