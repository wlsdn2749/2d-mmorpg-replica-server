using Grpc.Net.Client;
using GrpcGreeter;
using Mmorpg2d.Auth;

class Program
{
    private static Auth.AuthClient? _client;
    static async Task Main()
    {
        // 로컬 개발용: HTTP/2(무 TLS) 허용 스위치
        //AppContext.SetSwitch(
        //    "System.Net.Http.SocketsHttpHandler.Http2UnencryptedSupport", true);

        using var channel = GrpcChannel.ForAddress(
            "https://localhost:443",
            new GrpcChannelOptions
            {
                //Credentials = Grpc.Core.ChannelCredentials.Insecure
            });

        _client = new Auth.AuthClient(channel);

        while (true)
        {
            Console.WriteLine("\n===== 메뉴 =====");
            Console.WriteLine("[1] 회원가입");
            Console.WriteLine("[2] 로그인");
            Console.WriteLine("[0] 종료");
            Console.Write("선택: ");

            var key = Console.ReadLine();
            switch (key)
            {
                case "1":
                    await DoRegisterAsync();
                    break;
                case "2":
                    await DoLoginAsync();
                    break;
                case "0":
                    return;
                default:
                    Console.WriteLine("잘못된 입력입니다.");
                    break;
            }
        }
    }

    /* ---------- 회원가입 ---------- */
    private static async Task DoRegisterAsync()
    {
        Console.Write("이메일 입력    : ");
        var email = Console.ReadLine() ?? "";
        Console.Write("비밀번호     : ");
        var password = ReadPassword();

        var reply = await _client!.RegisterAsync(new RegisterRequest
        {
            Email = email,
            Password = password
        });

        Console.WriteLine($"[가입 결과] {reply.Success} / {reply.Detail}");
    }

    /* ---------- 로그인 ---------- */
    private static async Task DoLoginAsync()
    {
        Console.Write("아이디(이메일): ");
        var email = Console.ReadLine() ?? "";
        Console.Write("비밀번호     : ");
        var password = ReadPassword();

        var reply = await _client!.LoginAsync(new LoginRequest
        {
            Email = email,
            Password = password
        });

        Console.WriteLine($"[로그인 결과] {reply.Success} / {reply.Detail}");
        Console.WriteLine($"JWT: {reply.Jwt}");
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