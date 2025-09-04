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
            "http://localhost:8080",
            new GrpcChannelOptions
            {
                Credentials = Grpc.Core.ChannelCredentials.Insecure
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
            Console.WriteLine("[3] 이메일 확인");
            Console.WriteLine("\n===== TCP 게임서버 =====");
            Console.WriteLine("[q] JWT 검증:          --- 반드시 2번을 하고 해야함");
            Console.WriteLine("[w] 캐릭터 생성 :         --- Input 입력");
            Console.WriteLine("[e] 캐릭터 리스트 받기");
            Console.WriteLine("[r] 게임 접속 :          --- Index 입력");
            Console.WriteLine("[t] 상하좌우 움직이기:   --- 0,1,2,3 [상하좌우]");
            Console.WriteLine("[a] 공격 보내기 : --- 기본공격");
            Console.WriteLine("\n===== 인벤토리 테스트 =====");
            Console.WriteLine("[i] 인벤토리 조회하기");
            Console.WriteLine("[u] 퀵슬롯 사용 (1~9)");
            Console.WriteLine("[o] 슬롯 지정 아이템 사용");
            Console.WriteLine("\n===== 종료 로직 =====");
            Console.WriteLine("[x] 룸에서 나가기");
            Console.WriteLine("[z] 종료");
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
                    await AuthUtil.DoCheckEmailAsync(_client);
                    break;
                case "q":
                    await SessionManager.Instance.SendForEachJWTLoginAsync(_jwt);
                    break;
                case "w":
                    Console.Write("Username : ");
                    var username = Console.ReadLine();
                    await SessionManager.Instance.SendForEachCreateCharacterAsync(username);
                    break;
                case "e":
                    await SessionManager.Instance.SendForEachGetCharacterList();
                    break;
                case "r":
                    Console.Write("PlayerIdx : ");
                    int idx;
                    Int32.TryParse(Console.ReadLine(), out idx);
                    await SessionManager.Instance.SendForEachEnterGame(idx);
                    break;
                case "t":
                    Console.Write("Dir(상하좌우) 0,1,2,3:");
                    int dir;
                    Int32.TryParse(Console.ReadLine(), out dir);
                    await SessionManager.Instance.SendForEachMove(dir);
                    break;
                case "a":
                    await SessionManager.Instance.SendForEachAttack();
                    break;
                case "i":
                    await SessionManager.Instance.SendInventoryRequest();
                    break;
                case "u":
                    Console.Write("퀵슬롯 번호 (1~9): ");
                    int quickSlotNum;
                    if (Int32.TryParse(Console.ReadLine(), out quickSlotNum) && quickSlotNum >= 1 && quickSlotNum <= 9)
                    {
                        int quickSlotIndex = 30 + (quickSlotNum - 1); // 1->30, 2->31, ..., 9->38
                        await SessionManager.Instance.SendItemUseRequest(quickSlotIndex);
                    }
                    else
                    {
                        Console.WriteLine("잘못된 퀵슬롯 번호입니다. (1~9)");
                    }
                    break;
                case "o":
                    Console.Write("사용할 슬롯 번호 (0~39): ");
                    int slotIndex;
                    if (Int32.TryParse(Console.ReadLine(), out slotIndex))
                    {
                        await SessionManager.Instance.SendItemUseRequest(slotIndex);
                    }
                    else
                    {
                        Console.WriteLine("잘못된 슬롯 번호입니다.");
                    }
                    break;
                case "x":
                    await SessionManager.Instance.SendForLeave();
                    break;
                case "z":
                    return;
                default:
                    Console.WriteLine("잘못된 입력입니다.");
                    break;
            }
        }
    }

    static async Task ConnectToGameServer()
    {

        IPEndPoint endPoint = new IPEndPoint(IPAddress.Loopback, 6201);
        Connector connector = new Connector();

        connector.Connect(endPoint,
            () => {
                var session = SessionManager.Instance.Generate();
                return session;
            }, 1);
    }

    
}