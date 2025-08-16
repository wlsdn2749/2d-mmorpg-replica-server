#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

#include "AccountRepository.h"
#include "CharacterRepository.h"
#include "JwtAuth.h"
#include "Player.h"

#include "RoomManager.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];


bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	GConsoleLogger->WriteStdOut(Color::RED, L"Handle INVALID \n");
	return true;
}

bool Handle_C_JwtLoginRequest(PacketSessionRef& session, Protocol::C_JwtLoginRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	GConsoleLogger->WriteStdOut(Color::GREEN, L"Handle C_JwtLoginRequest \n");
	// 현재 상태가 Connected, 즉 연결되기 전이면 말이안되는거.
	if (gameSession->GetState() != GameSession::State::Connected) {
		// 서버 에러 반환
	}

	auto [eLoginResult, userId] = JwtAuth::Verify(pkt.accesstoken());
	
	if (eLoginResult == Protocol::ELoginResult::SUCCESS)
	{
		// TODO: 세션에 jwt의 데이터 저장?
		gameSession->_account = MakeShared<Account>();
		gameSession->_account->SetUserId(userId);

		// TODO: Accounts에 계정 정보 저장
		auto fut = AccountRepository::UpsertAccountAsync(userId);
	}

	Protocol::S_JwtLoginReply replyPkt;
	replyPkt.set_result(eLoginResult);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(sendBuffer);
	return true;
}

bool Handle_C_CreateCharacterRequest(PacketSessionRef& session, Protocol::C_CreateCharacterRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	auto userId = gameSession->_account->GetUserId();

	// userId는 1부터 시작하는데, 0이면 비정상 Validation
	ASSERT_CRASH(userId != 0)

	// 캐릭터 이름 검증 -> 
	// "이미 있는 이름이면" --> "등록된 이름입니다"
	// "모두 한글이 아니거나, 2-6글자가 아니면" -> 이름이 길거나 적합하지 않습니다.
	auto validResult = CharacterRepository::IsValidUsername(pkt.username());
	if (!validResult.isValid)
	{
		Protocol::S_CreateCharacterReply replyPkt;
		replyPkt.set_success(validResult.isValid);
		replyPkt.set_detail(validResult.message);
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(sendBuffer);
		return true;
	}

	// TODO: 지역 검증 
	// TODO: region 검증 

	// 검증 통과하면 진짜로 캐릭터 만들고 성공 리턴 
	// 여기서 실패할 수도 있으나, 나중에 판단..
	
	String username = StrToWstr(pkt.username());
	auto fut = CharacterRepository::CreateCharacterAsync(userId, username, pkt.gender(), pkt.region());

	Protocol::S_CreateCharacterReply replyPkt;
	replyPkt.set_success(true);
	replyPkt.set_detail("");
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(sendBuffer);
	return true;
}

bool Handle_C_CharacterListRequest(PacketSessionRef& session, Protocol::C_CharacterListRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	auto userId = gameSession->_account->GetUserId();

	auto fut = CharacterRepository::GetCharactersByUserAsync(userId);

	auto characters = fut.get();

	// TODO: Session에 Characters 저장
	for(const auto& character : characters)
	{
		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = character.characterId; // 나중에 CharacterId로 바꿔야함 DB에 있는
		playerRef->username = character.username; // utf8
		playerRef->posX = character.posX; // 나중에 posX로 바꿔야함 DB에 
		playerRef->posY = character.posY; // 나중에 posY로 바꿔야함 DB에
		playerRef->gender = character.gender;
		playerRef->region = character.region;
		playerRef->dir = character.dir;
		//playerRef->level = character.level; // TODO: 플레이어(캐릭터) 데이터 쪽으로 넘겨야함, level, exp, hp, mp
		
		playerRef->ownerSession = gameSession; // WeakPtr로 참조

		gameSession->_players.push_back(playerRef);
	}

	Protocol::S_CharacterListReply reply;
	auto* out = reply.mutable_characters();
	out->Reserve(static_cast<int>(characters.size()));
	for (const auto& m : characters) {
		Protocol::CharacterSummaryInfo* info = out->Add();
		info->set_username(m.username);
		info->set_level(1); // 나중에 이건, 플레이어 데이터에서 확충
		info->set_gender(m.gender);
		info->set_region(m.region);
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(reply);
	session->Send(sendBuffer);
	return true;
	
}

bool Handle_C_EnterGame(PacketSessionRef& session, Protocol::C_EnterGame& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	int index = pkt.playerindex();

	gameSession->_currentPlayer = gameSession->_players[index];
	

	// 입장할 룸 결정
	const int roomId = 0; // TODO: DB에서 받아 결정
	RoomRef room = RoomManager::Instance().Find(roomId);
	if (!room) {
		std::cout << "존재하지 않는 룸" << std::endl; 
	}

	room->DoAsync(&Room::Enter, gameSession->_currentPlayer); 
	

	gameSession->SetState(GameSession::State::InRoom);
	// Room에 들어왔음을 다른 플레이어에 알려야함 -- 이거 OnEnter()에서 진행함.
	// GRoom->DoAsync(&Room::Enter, gameSession->_currentPlayer);

	Protocol::S_EnterGame enterGamePkt;
	enterGamePkt.set_success(true);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_EnterGame]: Client가 Room에 접속 완료함 \n");

	return true;
}
bool Handle_C_LeaveGame(PacketSessionRef& session, Protocol::C_LeaveGame& pkt)
{
	return true;
}
bool Handle_C_PlayerMoveRequest(PacketSessionRef& session, Protocol::C_PlayerMoveRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if(gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if(!player) return false;

	RoomRef room = player->GetRoom();
	room->DoAsync([room, player, pkt] {
		room->OnRecvMoveReq(player, pkt);
	});

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_PlayerMoveReqeust]: Client가 Room에 이동요청함 \n");
	return true;
}
