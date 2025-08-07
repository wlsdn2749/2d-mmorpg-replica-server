using DummyClientCS.Utils;
using Grpc.Net.Client;
using GrpcGreeter;
using Mmorpg2d.Auth;
using ServerCore;
using System.Net;

class Program
{
    private static Auth.AuthClient? _client;
    static async Task Main()
    {
        using var channel = GrpcChannel.ForAddress(
            "https://mmo.jdj.kr:8443",
            new GrpcChannelOptions
            {
                //Credentials = Grpc.Core.ChannelCredentials.Insecure
            });

        _client = new Auth.AuthClient(channel);

        while (true)
        {
            Console.WriteLine("\n===== 메뉴 =====");
            Console.WriteLine("\n===== gRPC 인증서버=====");
            Console.WriteLine("[1] 회원가입");
            Console.WriteLine("[2] 로그인");
            Console.WriteLine("\n===== TCP 게임서버 =====");

            Console.WriteLine("[0] 종료");
            Console.Write("선택: ");

            var key = Console.ReadLine();
            switch (key)
            {
                case "1":
                    await AuthUtil.DoRegisterAsync(_client);
                    break;
                case "2":
                    await AuthUtil.DoLoginAsync(_client);
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
        int     port = 6100;

        IPAddress ipAddr = (await Dns.GetHostAddressesAsync(host))[0];
        IPEndPoint endPoint = new IPEndPoint(ipAddr, port);

        Connector connector = new Connector();

        connector.Connect(endPoint,
            () => { 
                // SessionManager로 생성
                // 세션 리턴
            })
    }

    
}