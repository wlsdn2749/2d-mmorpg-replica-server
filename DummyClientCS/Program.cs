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
    
    // State 변수 추가
    private static bool _isLoggedIn = false;        // 2번 완료
    private static bool _isJwtVerified = false;     // q 완료
    private static bool _isCharacterListLoaded = false; // e 완료
    private static bool _isInGame = false;          // r 완료
    
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
                    if (!string.IsNullOrEmpty(_jwt))
                    {
                        _isLoggedIn = true;
                        Console.WriteLine("로그인 완료! 이제 JWT 검증(q)을 진행할 수 있습니다.");
                    }
                    break;
                case "3":
                    await AuthUtil.DoCheckEmailAsync(_client);
                    break;
                case "q":
                    if (!_isLoggedIn)
                    {
                        Console.WriteLine("먼저 로그인(2번)을 완료해야 합니다!");
                        break;
                    }
                    await SessionManager.Instance.SendForEachJWTLoginAsync(_jwt);
                    _isJwtVerified = true;
                    Console.WriteLine("JWT 검증 완료! 이제 캐릭터 리스트 조회(e)를 진행할 수 있습니다.");
                    break;
                case "w":
                    Console.Write("Username : ");
                    var username = Console.ReadLine();
                    
                    // 유저네임 유효성 검사
                    if (string.IsNullOrWhiteSpace(username))
                    {
                        Console.WriteLine("유저네임은 비어있을 수 없습니다!");
                        break;
                    }
                    if (username.Length < 2 || username.Length > 20)
                    {
                        Console.WriteLine("유저네임은 2-20글자 사이여야 합니다!");
                        break;
                    }
                    if (!System.Text.RegularExpressions.Regex.IsMatch(username, @"^[a-zA-Z0-9가-힣]+$"))
                    {
                        Console.WriteLine("유저네임은 영문, 숫자, 한글만 사용 가능합니다!");
                        break;
                    }
                    
                    await SessionManager.Instance.SendForEachCreateCharacterAsync(username);
                    break;
                case "e":
                    if (!_isJwtVerified)
                    {
                        Console.WriteLine("먼저 JWT 검증(q)을 완료해야 합니다!");
                        break;
                    }
                    await SessionManager.Instance.SendForEachGetCharacterList();
                    _isCharacterListLoaded = true;
                    Console.WriteLine("캐릭터 리스트 조회 완료! 이제 게임 접속(r)을 진행할 수 있습니다.");
                    break;
                case "r":
                    if (!_isCharacterListLoaded)
                    {
                        Console.WriteLine("먼저 캐릭터 리스트 조회(e)를 완료해야 합니다!");
                        break;
                    }
                    Console.Write("PlayerIdx : ");
                    string? idxInput = Console.ReadLine();
                    
                    // PlayerIdx 유효성 검사
                    if (string.IsNullOrWhiteSpace(idxInput))
                    {
                        Console.WriteLine("PlayerIdx를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(idxInput, out int idx))
                    {
                        Console.WriteLine("PlayerIdx는 숫자여야 합니다!");
                        break;
                    }
                    if (idx < 0)
                    {
                        Console.WriteLine("PlayerIdx는 0 이상이어야 합니다!");
                        break;
                    }
                    
                    await SessionManager.Instance.SendForEachEnterGame(idx);
                    _isInGame = true;
                    Console.WriteLine("게임 접속 완료! 이제 게임 플레이 기능들(t,a,i,o,u)을 사용할 수 있습니다.");
                    break;
                case "t":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(r)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("Dir(상하좌우) 0,1,2,3:");
                    string? dirInput = Console.ReadLine();
                    
                    // 방향값 유효성 검사
                    if (string.IsNullOrWhiteSpace(dirInput))
                    {
                        Console.WriteLine("방향을 입력해주세요! (0=상, 1=하, 2=좌, 3=우)");
                        break;
                    }
                    if (!Int32.TryParse(dirInput, out int dir))
                    {
                        Console.WriteLine("방향은 숫자여야 합니다! (0=상, 1=하, 2=좌, 3=우)");
                        break;
                    }
                    if (dir < 0 || dir > 3)
                    {
                        Console.WriteLine("방향은 0~3 사이의 숫자여야 합니다! (0=상, 1=하, 2=좌, 3=우)");
                        break;
                    }
                    
                    await SessionManager.Instance.SendForEachMove(dir);
                    break;
                case "a":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(r)을 완료해야 합니다!");
                        break;
                    }
                    await SessionManager.Instance.SendForEachAttack();
                    break;
                case "i":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(r)을 완료해야 합니다!");
                        break;
                    }
                    await SessionManager.Instance.SendInventoryRequest();
                    break;
                case "u":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(r)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("퀵슬롯 번호 (1~9): ");
                    string? quickSlotInput = Console.ReadLine();
                    
                    // 퀵슬롯 번호 유효성 검사
                    if (string.IsNullOrWhiteSpace(quickSlotInput))
                    {
                        Console.WriteLine("퀵슬롯 번호를 입력해주세요! (1~9)");
                        break;
                    }
                    if (!Int32.TryParse(quickSlotInput, out int quickSlotNum))
                    {
                        Console.WriteLine("퀵슬롯 번호는 숫자여야 합니다! (1~9)");
                        break;
                    }
                    if (quickSlotNum < 1 || quickSlotNum > 9)
                    {
                        Console.WriteLine("퀵슬롯 번호는 1~9 사이여야 합니다!");
                        break;
                    }
                    
                    int quickSlotIndex = 30 + (quickSlotNum - 1); // 1->30, 2->31, ..., 9->38
                    await SessionManager.Instance.SendItemUseRequest(quickSlotIndex);
                    break;
                case "o":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(r)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("사용할 슬롯 번호 (0~39): ");
                    string? slotInput = Console.ReadLine();
                    
                    // 슬롯번호 유효성 검사
                    if (string.IsNullOrWhiteSpace(slotInput))
                    {
                        Console.WriteLine("슬롯 번호를 입력해주세요! (0~39)");
                        break;
                    }
                    if (!Int32.TryParse(slotInput, out int slotIndex))
                    {
                        Console.WriteLine("슬롯 번호는 숫자여야 합니다! (0~39)");
                        break;
                    }
                    if (slotIndex < 0 || slotIndex > 39)
                    {
                        Console.WriteLine("슬롯 번호는 0~39 사이여야 합니다! (0~29: 인벤토리, 30~39: 퀵슬롯)");
                        break;
                    }
                    
                    await SessionManager.Instance.SendItemUseRequest(slotIndex);
                    break;
                case "x":
                    if (!_isInGame)
                    {
                        Console.WriteLine("게임에 접속한 상태가 아닙니다!");
                        break;
                    }
                    await SessionManager.Instance.SendForLeave();
                    _isInGame = false;
                    Console.WriteLine("룸에서 나갔습니다. 게임을 다시 시작하려면 r을 눌러주세요.");
                    break;
                case "z":
                    Console.WriteLine("프로그램을 종료합니다.");
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