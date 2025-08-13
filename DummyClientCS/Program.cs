using DummyClientCS;
using DummyClientCS.Utils;
using Grpc.Net.Client;
using GrpcGreeter;
using Mmorpg2d.Auth;
using ServerCore;
using System.Net;

class Program
{
    private static Auth.AuthClient? _client;
    private static string? _jwt = "";
    static async Task Main()
    {
        using var channel = GrpcChannel.ForAddress(
            "https://mmo.jdj.kr:8443",
            new GrpcChannelOptions
            {
                //Credentials = Grpc.Core.ChannelCredentials.Insecure
            });

        _client = new Auth.AuthClient(channel);

        // GameServer TCP HandShake
        await ConnectToGameServer();


        while (true)
        {
            Console.WriteLine("\n===== 메뉴 =====");
            Console.WriteLine("\n===== gRPC 인증서버=====");
            Console.WriteLine("[1] 회원가입");
            Console.WriteLine("[2] 로그인");
            Console.WriteLine("\n===== TCP 게임서버 =====");
            Console.WriteLine("[3] JWT 검증");
            Console.WriteLine("[4] 캐릭터 생성");
            Console.WriteLine("[5] 캐릭터 리스트 받기");

            Console.WriteLine("[0] 종료");
            Console.Write("선택: ");

            var key = Console.ReadLine();

            SessionManager.Instance.SetCanSendPackets(true);
            switch (key)
            {
                case "1":
                    await AuthUtil.DoRegisterAsync(_client);
                    break;
                case "2":
                    _jwt = await AuthUtil.DoLoginAsync(_client);
                    break;
                case "3":
                    await SessionManager.Instance.SendForEachJWTLoginAsync(_jwt);
                    break;
                case "4":
                    Console.Write("Username : ");
                    var username = Console.ReadLine();
                    await SessionManager.Instance.SendForEachCreateCharacterAsync(username);
                    break;

                case "5":
                    await SessionManager.Instance.SendForEachGetCharacterList();
                    break;
                case "0":
                    return;
                default:
                    Console.WriteLine("잘못된 입력입니다.");
                    break;
            }
        }
    }

    static async Task ConnectToGameServer()
    {
        string  host = "mmo.jdj.kr";
        int     port = 6201;

        IPAddress ipAddr = (await Dns.GetHostAddressesAsync(host))[0];
        IPEndPoint endPoint = new IPEndPoint(ipAddr, port);

        Connector connector = new Connector();

        connector.Connect(endPoint,
            () => {
                var session = SessionManager.Instance.Generate();
                return session;
            }, 1);
    }

    
}