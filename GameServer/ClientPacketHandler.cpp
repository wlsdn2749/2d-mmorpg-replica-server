#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

#include "AccountRepository.h"
#include "CharacterRepository.h"
#include "JwtAuth.h"
#include "Player.h"

#include "RoomManager.h"
#include "ItemManager.h"

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

	gameSession->SetState(GameSession::State::InGame);

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

	int roomId = RoomManager::Instance().GetRoomIdByRegion(pkt.region());
	
	String username = StrToWstr(pkt.username());
	auto fut = CharacterRepository::CreateCharacterAsync(userId, username, pkt.gender(), pkt.region(), roomId);

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
		playerRef->gender = character.gender;
		playerRef->region = character.region;

		playerRef->core.id = character.characterId;
		playerRef->core.kind = EntityKind::Player;
		playerRef->core.pos = {character.posX, character.posY};
		playerRef->core.dir = character.dir; 
		//playerRef->SetHp(character) = character.level; // TODO: character가 hp, atk, level ... 등을 가져오도록 해야함 아직 안돼있음

		
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

bool Handle_C_DeleteCharacterRequest(PacketSessionRef& session, Protocol::C_DeleteCharacterRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// Room 접속 전에만 삭제 가능하도록 검증
	if (gameSession->GetState() != GameSession::State::Connected) {
		Protocol::S_DeleteCharacterReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_errormessage("게임 진행 중에는 캐릭터를 삭제할 수 없습니다.");
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(sendBuffer);
		return true;
	}

	auto userId = gameSession->_account->GetUserId();
	int characterIndex = pkt.characterindex();

	// 캐릭터 인덱스 범위 검증
	if (characterIndex < 0 || characterIndex >= gameSession->_players.size()) {
		Protocol::S_DeleteCharacterReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_errormessage("유효하지 않은 캐릭터입니다.");
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(sendBuffer);
		return true;
	}

	// 삭제하려는 캐릭터 정보 가져오기
	PlayerRef targetPlayer = gameSession->_players[characterIndex];
	int characterId = targetPlayer->playerId;

	// DB에서 캐릭터 삭제 (soft delete)
	auto fut = CharacterRepository::DeleteCharacterAsync(userId, characterId);
	bool deleteSuccess = fut.get();

	Protocol::S_DeleteCharacterReply replyPkt;
	if (deleteSuccess) {
		replyPkt.set_success(true);
		replyPkt.set_errormessage("");
		
		// 세션에서 캐릭터 제거
		gameSession->_players.erase(gameSession->_players.begin() + characterIndex);
		
		GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_DeleteCharacterRequest]: 캐릭터 삭제 성공 CharacterId[%d]\n", characterId);
	} else {
		replyPkt.set_success(false);
		replyPkt.set_errormessage("캐릭터 삭제에 실패했습니다.");
		
		GConsoleLogger->WriteStdOut(Color::RED, L"[C_DeleteCharacterRequest]: 캐릭터 삭제 실패 CharacterId[%d]\n", characterId);
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(sendBuffer);
	return true;
}

bool Handle_C_EnterGame(PacketSessionRef& session, Protocol::C_EnterGame& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	int index = pkt.playerindex();

	if (gameSession->_players.empty())
	{
		Protocol::S_EnterGame enterGamePkt;
		enterGamePkt.set_success(false);
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
		session->Send(sendBuffer);

		return false;
	}
	
	gameSession->_currentPlayer = gameSession->_players[index];
	PlayerRef player = gameSession->_currentPlayer;

	auto characterId = player->playerId;
	auto fut = CharacterRepository::GetCharacterStatsAsync(characterId);
	auto stat = fut.get();

	player->LoadCharacterStat(stat);

	const int roomId = player->LastRoomId();
	RoomRef room = RoomManager::Instance().Find(roomId);
	if (!room) 
		throw std::exception("Not Exsiting room");

	room->DoAsync(&Room::Enter, player); // 룸 입장 성공
	player->LoadInventoryFromDB(); // DB에서 인벤토리 로딩 (접속 시)

	
	gameSession->SetState(GameSession::State::InRoom);

	Protocol::S_EnterGame enterGamePkt;
	enterGamePkt.set_success(true);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_EnterGame]: Client가 Room에 접속 완료함 \n");

	return true;
}

bool Handle_C_LeaveGame(PacketSessionRef& session, Protocol::C_LeaveGame& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_currentPlayer;

	RoomRef room = player->GetRoom();

	bool result = false;
	string detail = "";
	switch (pkt.reason())
	{
		case Protocol::ELeaveReason::LEAVE_LOGOUT:
			result = gameSession->Logout(OUT detail); 
			break;
		case Protocol::ELeaveReason::LEAVE_CHANGE_CHARACTER:
			result = gameSession->CharacterSelect(OUT detail);
			break; 
		case Protocol::ELeaveReason::LEAVE_CHANGE_ROOM:// 클라가 직접 요청을 보낼때 이 항목은 사용하지 않음
			break;
		case Protocol::ELeaveReason::LEAVE_DISCONNECT: // 클라가 직접 요청을 보낼때 이 항목은 사용하지 않음
		default:
			GConsoleLogger->WriteStdOut(Color::GREEN, L"Change Room and disconnect is not used in C_LeaveGame"); 
			break;
	}
	
	Protocol::S_LeaveGame leaveGamePkt;
	leaveGamePkt.set_success(result);
	leaveGamePkt.set_detail(detail);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(leaveGamePkt);
	session->Send(sendBuffer);
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

	return true;
}

bool Handle_C_ChangeRoomReady(PacketSessionRef& session, Protocol::C_ChangeRoomReady& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	
	if(gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player) return false;
	

	RoomRef room = player->GetRoom();
	//room->DoAsync(&Room::ChangeRoomReady, player, pkt);

	room->DoAsync([room, player, pkt] 
	{
		room->ChangeRoomReady(player, pkt); 
	});

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_ChangeRoomReady]: Client가 룸 이동 준비요청함 \n");
}

bool Handle_C_PlayerAttackRequest(PacketSessionRef& session, Protocol::C_PlayerAttackRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player) return false;


	RoomRef room = player->GetRoom();
	room->DoAsync([room, player, pkt]
	{
		room->OnRecvAttackReq(player, pkt);
	});
	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_PlayerAttackRequest]: Player가 공격 요청함 \n");
	return true;
}

bool Handle_C_InventoryRequest(PacketSessionRef& session, Protocol::C_InventoryRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player) 
		return false;

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_InventoryRequest]: Player가 인벤토리 조회 요청함 \n");

	// 플레이어의 인벤토리 정보를 가져와서 응답 패킷 생성
	const InventorySystem& inventory = player->GetInventory();
	auto slots = inventory.ToProtocolSlots();

	Protocol::S_InventoryReply replyPkt;
	for (const auto& slotInfo : slots)
	{
		*replyPkt.add_slots() = slotInfo;
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_C_ItemUseRequest(PacketSessionRef& session, Protocol::C_ItemUseRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	int slotIndex = pkt.slotindex();
	
	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_ItemUseRequest]: Player가 슬롯[%d] 아이템 사용 요청함 \n", slotIndex);

	// 아이템 사용 처리
	EUseItemResult result = player->UseItem(slotIndex);

	Protocol::S_ItemUseReply replyPkt;
	replyPkt.set_success(result == EUseItemResult::Success);

	// 실패 시 에러 메시지 설정
	switch (result)
	{
		case EUseItemResult::Success:
			replyPkt.set_errormessage("");
			break;
		case EUseItemResult::ItemNotFound:
			replyPkt.set_errormessage("아이템을 찾을 수 없습니다.");
			break;
		case EUseItemResult::ItemNotUsable:
			replyPkt.set_errormessage("사용할 수 없는 아이템입니다.");
			break;
		case EUseItemResult::CooldownActive:
			replyPkt.set_errormessage("아이템이 쿨다운 중입니다.");
			break;
		case EUseItemResult::InvalidCondition:
			replyPkt.set_errormessage("사용 조건을 만족하지 않습니다.");
			break;
		default:
			replyPkt.set_errormessage("알 수 없는 오류가 발생했습니다.");
			break;
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(sendBuffer);

	// 아이템 사용 성공 시 인벤토리 업데이트 브로드캐스트
	if (result == EUseItemResult::Success)
	{
		// Room에 있는 모든 플레이어에게 인벤토리 변경 사항을 브로드캐스트
		RoomRef room = player->GetRoom();
		if (room)
		{
			room->DoAsync([room, player]()
			{
				Protocol::S_InventoryUpdate updatePkt;
				auto slots = player->GetInventory().ToProtocolSlots();
				for (const auto& slotInfo : slots)
				{
					*updatePkt.add_changedslots() = slotInfo;
				}

				auto sendBuffer = ClientPacketHandler::MakeSendBuffer(updatePkt);
				
				// 해당 플레이어에게만 전송 (인벤토리는 개인 정보)
				if (auto gameSession = player->ownerSession.lock())
				{
					gameSession->Send(sendBuffer);
				}
			});
		}
	}

	return true;
}
