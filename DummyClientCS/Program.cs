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
            Console.WriteLine("[4] JWT 검증:          --- 반드시 2번을 하고 해야함");
            Console.WriteLine("[5] 캐릭터 생성 :         --- Input 입력");
            Console.WriteLine("[6] 캐릭터 리스트 받기");
            Console.WriteLine("[7] 캐릭터 삭제 :         --- Index 입력");
            Console.WriteLine("[8] 게임 접속 :          --- Index 입력");
            Console.WriteLine("[9] 상하좌우 움직이기:   --- 0,1,2,3 [상하좌우]");
            Console.WriteLine("[0] 공격 보내기 : --- 기본공격");
            Console.WriteLine("\n===== 인벤토리 테스트 =====");
            Console.WriteLine("[i] 인벤토리 조회하기");
            Console.WriteLine("[u] 퀵슬롯 사용 (1~9)");
            Console.WriteLine("[o] 슬롯 지정 아이템 사용");
            Console.WriteLine("[g] 아이템 지급 테스트 (서버에서 아이템 받기)");
            Console.WriteLine("\n===== 채팅 테스트 =====");
            Console.WriteLine("[t] 룸 채팅 (현재 룸의 플레이어들에게만)");
            Console.WriteLine("[y] 전체 채팅 (모든 룸의 플레이어들에게)");
            Console.WriteLine("\n===== Death 시스템 테스트 =====");
            Console.WriteLine("[p] 리스폰 요청 (사망 후 부활하기)");
            Console.WriteLine("\n===== 파티 시스템 테스트 =====");
            Console.WriteLine("[a] 파티 초대 (PlayerId 입력)");
            Console.WriteLine("[s] 파티 초대 수락");
            Console.WriteLine("[f] 파티 초대 거절");
            Console.WriteLine("[q] 파티 탈퇴");
            Console.WriteLine("[w] 파티원 강퇴 (PlayerId 입력)");
            Console.WriteLine("\n===== 공개 파티 시스템 (56-63번) =====");
            Console.WriteLine("[v] 공개 파티 생성 (파티명 입력)");
            Console.WriteLine("[b] 전체 파티 목록 조회");
            Console.WriteLine("[n] 파티 가입 요청 (PartyId 입력)");
            Console.WriteLine("[j] 리더: 가입 요청 리스트 조회");
            Console.WriteLine("[m] 리더: 가입 요청 수락");
            Console.WriteLine("[k] 리더: 가입 요청 거절");
            Console.WriteLine("\n===== 종료 로직 테스트 =====");
            Console.WriteLine("[x] 룸에서 나가기 (캐릭터 변경)");
            Console.WriteLine("[l] 로그아웃 테스트 (JWT 인증 상태로 복귀)");
            Console.WriteLine("[c] 캐릭터 변경 테스트");  
            Console.WriteLine("[r] 룸 이동 테스트");
            Console.WriteLine("[d] 연결 해제 테스트");
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
                case "4":
                    if (!_isLoggedIn)
                    {
                        Console.WriteLine("먼저 로그인(2번)을 완료해야 합니다!");
                        break;
                    }
                    await SessionManager.Instance.SendForEachJWTLoginAsync(_jwt);
                    _isJwtVerified = true;
                    Console.WriteLine("JWT 검증 완료! 이제 캐릭터 리스트 조회(6번)를 진행할 수 있습니다.");
                    break;
                case "5":
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
                case "6":
                    if (!_isJwtVerified)
                    {
                        Console.WriteLine("먼저 JWT 검증(4번)을 완료해야 합니다!");
                        break;
                    }
                    await SessionManager.Instance.SendForEachGetCharacterList();
                    _isCharacterListLoaded = true;
                    Console.WriteLine("캐릭터 리스트 조회 완료! 이제 게임 접속(8번) 또는 캐릭터 삭제(7번)를 진행할 수 있습니다.");
                    break;
                case "7":
                    if (!_isCharacterListLoaded)
                    {
                        Console.WriteLine("먼저 캐릭터 리스트 조회(6번)를 완료해야 합니다!");
                        break;
                    }
                    if (_isInGame)
                    {
                        Console.WriteLine("게임 진행 중에는 캐릭터를 삭제할 수 없습니다! 먼저 룸에서 나가세요(x).");
                        break;
                    }
                    Console.Write("삭제할 캐릭터 인덱스 : ");
                    string? deleteIdxInput = Console.ReadLine();
                    
                    // 캐릭터 인덱스 유효성 검사
                    if (string.IsNullOrWhiteSpace(deleteIdxInput))
                    {
                        Console.WriteLine("캐릭터 인덱스를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(deleteIdxInput, out int deleteIdx))
                    {
                        Console.WriteLine("캐릭터 인덱스는 숫자여야 합니다!");
                        break;
                    }
                    if (deleteIdx < 0)
                    {
                        Console.WriteLine("캐릭터 인덱스는 0 이상이어야 합니다!");
                        break;
                    }
                    
                    Console.WriteLine($"정말로 캐릭터 인덱스 {deleteIdx}를 삭제하시겠습니까? (y/N):");
                    string? confirm = Console.ReadLine();
                    if (confirm?.ToLower() == "y" || confirm?.ToLower() == "yes")
                    {
                        await SessionManager.Instance.SendDeleteCharacterRequest(deleteIdx);
                    }
                    else
                    {
                        Console.WriteLine("캐릭터 삭제를 취소했습니다.");
                    }
                    break;
                case "8":
                    if (!_isCharacterListLoaded)
                    {
                        Console.WriteLine("먼저 캐릭터 리스트 조회(6번)를 완료해야 합니다!");
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
                    Console.WriteLine("게임 접속 완료! 이제 게임 플레이 기능들(9,0,i,o,u)을 사용할 수 있습니다.");
                    break;
                case "9":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
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
                case "0":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    await SessionManager.Instance.SendForEachAttack();
                    break;
                case "i":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    await SessionManager.Instance.SendInventoryRequest();
                    break;
                case "u":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
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
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
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
                case "g":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("아이템 ID 입력 (예: 10001=체력포션): ");
                    string? itemIdInput = Console.ReadLine();

                    // 아이템ID 유효성 검사
                    if (string.IsNullOrWhiteSpace(itemIdInput))
                    {
                        Console.WriteLine("아이템 ID를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(itemIdInput, out int itemId))
                    {
                        Console.WriteLine("아이템 ID는 숫자여야 합니다!");
                        break;
                    }
                    if (itemId <= 0)
                    {
                        Console.WriteLine("아이템 ID는 0보다 큰 숫자여야 합니다!");
                        break;
                    }

                    Console.Write("수량 입력 (예: 5): ");
                    string? countInput = Console.ReadLine();

                    // 수량 유효성 검사
                    if (string.IsNullOrWhiteSpace(countInput))
                    {
                        Console.WriteLine("수량을 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(countInput, out int count))
                    {
                        Console.WriteLine("수량은 숫자여야 합니다!");
                        break;
                    }
                    if (count <= 0)
                    {
                        Console.WriteLine("수량은 0보다 큰 숫자여야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendGiveItemRequest(itemId, count);
                    break;
                case "t":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("룸 채팅 메시지 입력: ");
                    string? roomChatMessage = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(roomChatMessage))
                    {
                        Console.WriteLine("메시지를 입력해주세요!");
                        break;
                    }

                    await SessionManager.Instance.SendRoomChat(roomChatMessage);
                    break;
                case "y":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("전체 채팅 메시지 입력: ");
                    string? allChatMessage = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(allChatMessage))
                    {
                        Console.WriteLine("메시지를 입력해주세요!");
                        break;
                    }

                    await SessionManager.Instance.SendAllChat(allChatMessage);
                    break;
                case "p":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }

                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine("========================================");
                    Console.WriteLine("[Death 시스템 테스트] 리스폰 요청");
                    Console.WriteLine("========================================");
                    Console.WriteLine("주의: 이 기능은 사망 상태에서만 작동합니다.");
                    Console.WriteLine("사용법:");
                    Console.WriteLine("   1. 몬스터에게 공격받아 사망하세요");
                    Console.WriteLine("   2. S_BroadcastPlayerDeath 메시지가 나타나면");
                    Console.WriteLine("   3. 'p' 키를 눌러 리스폰을 요청하세요");
                    Console.WriteLine("   4. S_PlayerDeathCommit으로 리스폰이 승인됩니다");
                    Console.WriteLine("   5. S_ChangeRoomCommit으로 목적지 맵으로 이동합니다");
                    Console.WriteLine("========================================");
                    Console.ResetColor();

                    await SessionManager.Instance.SendPlayerDeathReady();
                    break;
                case "a":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("초대할 플레이어 ID 입력: ");
                    string? inviteTargetInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(inviteTargetInput))
                    {
                        Console.WriteLine("플레이어 ID를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(inviteTargetInput, out int targetPid))
                    {
                        Console.WriteLine("플레이어 ID는 숫자여야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyInviteRequest(targetPid);
                    break;
                case "s":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("수락할 파티 ID 입력: ");
                    string? acceptPartyIdInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(acceptPartyIdInput))
                    {
                        Console.WriteLine("파티 ID를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(acceptPartyIdInput, out int acceptPartyId))
                    {
                        Console.WriteLine("파티 ID는 숫자여야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyInviteResponse(acceptPartyId, true);
                    break;
                case "f":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("거절할 파티 ID 입력: ");
                    string? rejectPartyIdInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(rejectPartyIdInput))
                    {
                        Console.WriteLine("파티 ID를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(rejectPartyIdInput, out int rejectPartyId))
                    {
                        Console.WriteLine("파티 ID는 숫자여야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyInviteResponse(rejectPartyId, false);
                    break;
                case "q":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyLeave();
                    break;
                case "w":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("강퇴할 플레이어 ID 입력: ");
                    string? kickTargetInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(kickTargetInput))
                    {
                        Console.WriteLine("플레이어 ID를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(kickTargetInput, out int kickTargetPid))
                    {
                        Console.WriteLine("플레이어 ID는 숫자여야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyKick(kickTargetPid);
                    break;
                case "v":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("생성할 파티명 입력: ");
                    string? partyName = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(partyName))
                    {
                        Console.WriteLine("파티명을 입력해주세요!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyCreateRequest(partyName);
                    break;
                case "b":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyListRequest();
                    break;
                case "n":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("가입할 파티 ID 입력: ");
                    string? joinPartyIdInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(joinPartyIdInput))
                    {
                        Console.WriteLine("파티 ID를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(joinPartyIdInput, out int joinPartyId))
                    {
                        Console.WriteLine("파티 ID는 숫자여야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyJoinRequest(joinPartyId);
                    break;
                case "j":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("[리더] 조회할 파티 ID 입력: ");
                    string? queryPartyIdInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(queryPartyIdInput))
                    {
                        Console.WriteLine("파티 ID를 입력해주세요!");
                        break;
                    }
                    if (!Int32.TryParse(queryPartyIdInput, out int queryPartyId))
                    {
                        Console.WriteLine("파티 ID는 숫자여야 합니다!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyJoinRequestListQuery(queryPartyId);
                    break;
                case "m":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("[리더] 파티 ID 입력: ");
                    string? acceptResponsePartyIdInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(acceptResponsePartyIdInput) || !Int32.TryParse(acceptResponsePartyIdInput, out int acceptResponsePartyId))
                    {
                        Console.WriteLine("올바른 파티 ID를 입력해주세요!");
                        break;
                    }

                    Console.Write("[리더] 수락할 요청자 PlayerId 입력: ");
                    string? acceptRequesterInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(acceptRequesterInput) || !Int32.TryParse(acceptRequesterInput, out int acceptRequesterPid))
                    {
                        Console.WriteLine("올바른 PlayerId를 입력해주세요!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyJoinResponseWithPid(acceptResponsePartyId, acceptRequesterPid, true);
                    break;
                case "k":
                    if (!_isInGame)
                    {
                        Console.WriteLine("먼저 게임 접속(8번)을 완료해야 합니다!");
                        break;
                    }
                    Console.Write("[리더] 파티 ID 입력: ");
                    string? rejectResponsePartyIdInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(rejectResponsePartyIdInput) || !Int32.TryParse(rejectResponsePartyIdInput, out int rejectResponsePartyId))
                    {
                        Console.WriteLine("올바른 파티 ID를 입력해주세요!");
                        break;
                    }

                    Console.Write("[리더] 거절할 요청자 PlayerId 입력: ");
                    string? rejectRequesterInput = Console.ReadLine();

                    if (string.IsNullOrWhiteSpace(rejectRequesterInput) || !Int32.TryParse(rejectRequesterInput, out int rejectRequesterPid))
                    {
                        Console.WriteLine("올바른 PlayerId를 입력해주세요!");
                        break;
                    }

                    await SessionManager.Instance.SendPartyJoinResponseWithPid(rejectResponsePartyId, rejectRequesterPid, false);
                    break;
                case "x":
                    if (!_isInGame)
                    {
                        Console.WriteLine("게임에 접속한 상태가 아닙니다!");
                        break;
                    }
                    await SessionManager.Instance.SendLeaveForCharacterChange();
                    _isInGame = false;
                    Console.WriteLine("✅ 캐릭터 변경으로 룸에서 나갔습니다. 게임을 다시 시작하려면 8번을 눌러주세요.");
                    break;
                case "l":
                    if (!_isInGame)
                    {
                        Console.WriteLine("게임에 접속한 상태가 아닙니다!");
                        break;
                    }
                    await SessionManager.Instance.SendLeaveForLogout();
                    _isInGame = false;
                    _isCharacterListLoaded = false;
                    _isJwtVerified = false;
                    Console.WriteLine("✅ 로그아웃 테스트 완료. JWT 인증 상태로 복귀했습니다. 다시 시작하려면 4번부터 진행하세요.");
                    break;
                case "c":
                    if (!_isInGame)
                    {
                        Console.WriteLine("게임에 접속한 상태가 아닙니다!");
                        break;
                    }
                    await SessionManager.Instance.SendLeaveForCharacterChange();
                    _isInGame = false;
                    Console.WriteLine("✅ 캐릭터 변경 테스트 완료. 캐릭터 선택창으로 복귀했습니다. 게임을 다시 시작하려면 8번을 눌러주세요.");
                    break;
                case "r":
                    if (!_isInGame)
                    {
                        Console.WriteLine("게임에 접속한 상태가 아닙니다!");
                        break;
                    }
                    await SessionManager.Instance.SendLeaveForRoomChange();
                    Console.WriteLine("✅ 룸 이동 테스트 완료. (현재 미구현 상태)");
                    break;
                case "d":
                    if (!_isInGame)
                    {
                        Console.WriteLine("게임에 접속한 상태가 아닙니다!");
                        break;
                    }
                    await SessionManager.Instance.SendLeaveForDisconnect();
                    _isInGame = false;
                    Console.WriteLine("✅ 연결 해제 테스트 완료. 디버그 로깅용입니다.");
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