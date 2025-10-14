#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

#include "AccountRepository.h"
#include "CharacterRepository.h"
#include "JwtAuth.h"
#include "Player.h"

#include "RoomManager.h"
#include "ItemManager.h"

#include "EquipmentManager.h"

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
	if (gameSession->GetState() != GameSession::State::InGame) {
		Protocol::S_DeleteCharacterReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Cannot Delete while state is not State::InGame.");
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
		replyPkt.set_errormessage("Invalid Character ex) Idx is wrong");
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
	player->LoadEquipmentFromDB(); // DB에서 장비 로딩 (접속 시)


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

	// 아이템 정보를 사용 전에 미리 저장 (RemoveItem으로 정보가 변경되기 전)
	int itemId = player->GetItemIdFromSlot(slotIndex);

	// 아이템 사용 처리
	EUseItemResult result = player->UseItem(slotIndex); // 여기서 감소까지 처리

	// 성공 시 아이템 효과 적용
	if (result == EUseItemResult::Success && itemId > 0) {
		ItemManager::Instance().ApplyItemEffect(itemId, 1, player);
	}

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

bool Handle_C_NpcInteractRequest(PacketSessionRef& session, Protocol::C_NpcInteractRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;

	RoomRef room = player->GetRoom();

	// InteractionType에 맞게 Room에 실행 넘기기
	room->DoAsync(&Room::HandleNpcInteract, player, pkt.interactiontype());
	
}
bool Handle_C_NpcShopBuyRequest(PacketSessionRef& session, Protocol::C_NpcShopBuyRequest& pkt)
{

	// TODO
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;

	RoomRef room = player->GetRoom();

	return false;
}

bool Handle_C_PlayerDeathReady(PacketSessionRef& session, Protocol::C_PlayerDeathReady& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	RoomRef room = player->GetRoom();
	if (!room)
		return false;

	// PlayerDeathReady 호출 - 실제 리스폰 처리
	room->DoAsync(&Room::PlayerDeathReady, player);

	return true;
}

bool Handle_C_PlayerChat(PacketSessionRef& session, Protocol::C_PlayerChat& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;
	
	PlayerRef player = gameSession->_currentPlayer;

	RoomRef room = player->GetRoom();

	auto playerChatInfo = pkt.playerchatinfo();
	switch (playerChatInfo.chattype())
	{
		case Protocol::EChatType::CHAT_ROOM:
			room->DoAsync(&Room::AddChat, gameSession, pkt);
			break;
		case Protocol::EChatType::CHAT_ALL:
			RoomManager::Instance().DoAsyncForAllRooms(&Room::AddChat, gameSession, pkt);
			break;
		[[unlikely]] default:
			ASSERT_CRASH("WRONG VALUE");
	}

	return true;
}

bool Handle_C_GiveItemRequest(PacketSessionRef& session, Protocol::C_GiveItemRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	Protocol::S_GiveItemReply replyPkt;

	// 아이템 유효성 검사
	int itemId = pkt.itemid();
	int count = pkt.count();

	if (itemId <= 0 || count <= 0)
	{
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Wrong Item Infomation");
	}
	else
	{
		// 아이템 지급 시도
		auto result = player->AddItem(itemId, count);
		if (result == EAddItemResult::Success)
		{
			replyPkt.set_success(true);
			replyPkt.set_errormessage("");

			auto slots = player->GetInventory().ToProtocolSlots();
			for (const auto& slot : slots)
			{
				if (slot.itemid() == itemId)
				{
					*replyPkt.mutable_addedslot() = slot;
					break;
				}
			}

			GConsoleLogger->WriteStdOut(Color::GREEN,
				L"[Test] To Player %d, Item:%d, Count:%d Give Complete\n",
				player->playerId, itemId, count);
		}
		else
		{
			replyPkt.set_success(false);
			replyPkt.set_errormessage("Failed to give item ex) (Inventory Full)..");
		}
	}

	auto replyBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(replyBuffer);

	return true;
}

bool Handle_C_PartyInviteRequest(PacketSessionRef& session, Protocol::C_PartyInviteRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	// 초대한 유저가 이미 
	// 파티가 있는 경우: 그 파티
	// 없는 경우:		 새로 만든 파티
	auto party = PartyManager::Instance().CreateParty(player);
	
	// 특정 Id를 가진 유저에게, 정보를 띄워주어야함
	// PlayerRef 찾기
	auto targetPlayer = RoomManager::Instance().FindPlayerInAllRooms(pkt.targetpid());
	Protocol::S_PartyInviteReply replyPkt;
	if (!targetPlayer)
	{
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Player not found");
		gameSession->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));
		return false;
	}

	// Room 찾기
	auto targetRoom = targetPlayer->GetRoom();
	if (!targetRoom)
	{
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Player not in room");
		gameSession->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));
		return false;
	}


	replyPkt.set_success(true);
	replyPkt.set_errormessage("");


	// 서버 -> 클라 : 파티 초대 요청 결과 전송
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	gameSession->Send(sendBuffer);
	
	// invitee에게 알림 전달
	targetRoom->DoAsync(&Room::HandlePartyInvite, player, targetPlayer);

	return true;
}

bool Handle_C_PartyInviteResponse(PacketSessionRef& session, Protocol::C_PartyInviteResponse& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef invitee = gameSession->_currentPlayer;
	if (!invitee)
		return false;

	auto accept = pkt.accept();
	if (!accept)
		return false;

	RoomRef room = invitee->GetRoom();
	
	room->DoAsync(&Room::HandlePartyInviteResponse, invitee, pkt.partyid(), pkt.accept());
	return true;

}

bool Handle_C_PartyLeave(PacketSessionRef& session, Protocol::C_PartyLeave& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	RoomRef room = player->GetRoom();
	if (!room)
		return false;

	if (pkt.has_selfleave())
	{
		room->DoAsync(&Room::HandlePartyLeave, player, true, player->playerId);
	}
	else if(pkt.has_targetpid())
	{
		int32 partyId = player->GetPartyId();
		if (partyId == 0)
		{
			GConsoleLogger->WriteStdOut(Color::RED, L"[C_PartyLeave] Player not in party");
			return false;
		}

		auto targetPlayer = RoomManager::Instance().FindPlayerInAllRooms(pkt.targetpid());
		if (!targetPlayer)
		{
			GConsoleLogger->WriteStdOut(Color::RED, L"[C_PartyLeave] Target player not found: %d", pkt.targetpid());
			return false;
		}

		room->DoAsync(&Room::HandlePartyLeave, player, false, pkt.targetpid());

		GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_PartyLeave] Player %d kicked player %d",
			player->playerId, targetPlayer->playerId);
	}
	else
	{
		CRASH(); // 둘 중 하나는 반드시 있어야함
	}
	
	return true;

}

bool Handle_C_PartyCreateRequest(PacketSessionRef& session, Protocol::C_PartyCreateRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	RoomRef room = player->GetRoom();
	if (!room)
		return false;

	if (!player || player->IsInParty())
	{
		Protocol::S_PartyCreateReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_message("Already in party");

		auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(SendBuffer);
		return false;
	}

	PartyRef party = PartyManager::Instance().CreatePartyWithName(player, pkt.partyname());

	Protocol::S_PartyCreateReply replyPkt;
	replyPkt.set_success(party != nullptr);
	replyPkt.set_message(party ? "Party created" : "Failed to create party");

	auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(SendBuffer);

	return true;
}

bool Handle_C_PartyJoinRequest(PacketSessionRef& session, Protocol::C_PartyJoinRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	RoomRef room = player->GetRoom();
	if (!room)
		return false;

	PartyRef party = PartyManager::Instance().FindParty(pkt.partyid());

	if (!party)
	{
		// 파티가 존재하지 않음
		Protocol::S_PartyJoinReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_message("Party not found");
		auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(SendBuffer);
		return false;
	}

	if (party->IsFull())
	{
		Protocol::S_PartyJoinReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_message("Party is full");
		auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(SendBuffer);
		return false;
	}

	bool added = PartyManager::Instance().AddJoinRequest(pkt.partyid(), player);

	Protocol::S_PartyJoinReply replyPkt;
	replyPkt.set_success(added);
	replyPkt.set_message(added ? "Request sent to leader" : "Request already pending");
	auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(SendBuffer);

	// 리더에게 알림 전송
	if (added)
	{
		PlayerRef leader = party->GetLeader();

		if (auto leaderSession = leader->ownerSession.lock())
		{
			Protocol::S_PartyJoinNotify notifyPkt;
			notifyPkt.set_joinplayerid(player->playerId);
			notifyPkt.set_partyid(pkt.partyid());
			notifyPkt.set_leaderid(leader->playerId);

			auto SendBuffer = ClientPacketHandler::MakeSendBuffer(notifyPkt);
			leaderSession->Send(SendBuffer);
		}
	}

	return true;
}

bool Handle_C_PartyJoinResponse(PacketSessionRef& session, Protocol::C_PartyJoinResponse& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	RoomRef room = player->GetRoom();
	if (!room)
		return false;

	PartyRef party = PartyManager::Instance().FindParty(pkt.partyid());

	if (!party || party->GetLeader() != player)
	{
		return false; // 리더가 아니거나 파티가 없음
	}

	// 변경: requesterPid로 특정 요청자 찾기
	PlayerRef requester = PartyManager::Instance().FindRequesterById(pkt.partyid(), pkt.requesterpid());
	
	// 요청자가 없으면 종료
	if (!requester)
		return true;

	if (pkt.accept())
	{
		// 수락: 파티에 가입시킴
		bool success = PartyManager::Instance().JoinParty(pkt.partyid(), requester);

		// Option 요청자에게 결과 전송
		if (success)
		{
			PartyManager::Instance().RemoveJoinRequest(pkt.partyid(), requester);
		}
	}
	else
	{
		// 거절: 요청 제거
		PartyManager::Instance().RemoveJoinRequest(pkt.partyid(), requester);
	}

	return true;
}

bool Handle_C_PartyList(PacketSessionRef& session, Protocol::C_PartyList& pkt)
{
	Vector<Protocol::PartyInfo> parties = PartyManager::Instance().GetAllPublicParties();

	Protocol::S_PartyList replyPkt;
	for (auto& partyInfo : parties)
	{
		*replyPkt.add_partyinfos() = partyInfo;
	}

	auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(SendBuffer);
	return true;
}

bool Handle_C_PartyJoinRequestList(PacketSessionRef& session, Protocol::C_PartyJoinRequestList& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	PartyRef party = PartyManager::Instance().FindParty(pkt.partyid());

	if (!party || party->GetLeader() != player)
	{
		return false; // 리더가 아니면 조회 불가
	}

	// 요청자 리스트 가져오기
	Vector<PlayerRef> requesters = PartyManager::Instance().GetJoinRequesters(pkt.partyid());

	Protocol::S_PartyJoinRequestList replyPkt;
	replyPkt.set_partyid(pkt.partyid());

	for (const auto& requester : requesters)
	{
		if (!requester) continue;

		auto* info = replyPkt.add_requesters();
		info->set_playerid(requester->playerId);
		info->set_playername(requester->username);
		info->set_level(requester->Level());
	}

	auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(SendBuffer);

	return true;
}

bool Handle_C_EquipItemRequest(PacketSessionRef& session, Protocol::C_EquipItemRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	int inventorySlotIndex = pkt.inventoryslotindex();

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_EquipItemRequest]: Player requests to equip item from slot[%d]\n", inventorySlotIndex);

	// 1. 인벤토리에서 장비 아이템 및 인스턴스 ID 확인
	const ItemSlot& invSlot = player->GetInventory().GetSlot(inventorySlotIndex);
	int itemId = invSlot.itemId;
	int equipmentInstanceId = invSlot.equipmentInstanceId;

	if (itemId == 0)
	{
		Protocol::S_EquipItemReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Empty inventory slot");
		session->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));
		return true;
	}

	// 2. 장비 타입 확인
	Protocol::EItemType itemType = ItemManager::Instance().GetItemType(itemId);
	if (itemType != Protocol::EItemType::ITEM_TYPE_EQUIPMENT)
	{
		Protocol::S_EquipItemReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Item is not equipment");
		session->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));
		return true;
	}

	// 3. 슬롯 타입 확인
	Protocol::EEquipmentSlotType slotType = EquipmentManager::Instance().GetSlotType(itemId);

	// 4. 레벨 체크
	if (!EquipmentManager::Instance().CanEquip(itemId, player->Level()))
	{
		Protocol::S_EquipItemReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Level requirement not met");
		session->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));
		return true;
	}

	// 5. 기존 장비 확인 및 인벤토리 반환
	int returnedSlotIndex = -1;
	bool hasOldEquipment = false;
	int oldItemId = 0;
	int oldEquipmentInstanceId = 0;
	int oldEnhancementLevel = 0;

	if (!player->GetEquipment().IsSlotEmpty(slotType))
	{
		oldItemId = player->GetEquipmentItemIdFromSlot(slotType);
		oldEquipmentInstanceId = player->GetEquipmentInstanceIdFromSlot(slotType);
		oldEnhancementLevel = player->GetEquipment().GetEquipmentSlot(slotType).enhancementLevel;

		// 기존 장비를 인벤토리에 반환 (equipmentInstanceId 보존)
		EAddItemResult addResult = player->AddItem(oldItemId, 1, oldEquipmentInstanceId);
		if (addResult != EAddItemResult::Success)
		{
			Protocol::S_EquipItemReply replyPkt;
			replyPkt.set_success(false);
			replyPkt.set_errormessage("Inventory is full");
			session->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));
			return true;
		}

		// 반환된 슬롯 찾기
		for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
		{
			const ItemSlot& slot = player->GetInventory().GetSlot(i);
			if (slot.equipmentInstanceId == oldEquipmentInstanceId)
			{
				returnedSlotIndex = i;
				break;
			}
		}

		hasOldEquipment = true;
	}

	// 6. 인벤토리에서 제거 (장착하기 전에 먼저 제거)
	ERemoveItemResult removeResult = player->RemoveItem(inventorySlotIndex, 1, false); // Instance 보존
	if (removeResult != ERemoveItemResult::Success)
	{
		// 롤백: 기존 장비를 인벤토리에서 제거하고 다시 장착
		if (hasOldEquipment)
		{
			player->RemoveItem(returnedSlotIndex, 1, false); // Instance 보존
			player->EquipItem(slotType, oldItemId, oldEquipmentInstanceId, oldEnhancementLevel);
		}
		Protocol::S_EquipItemReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Failed to remove from inventory");
		session->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));
		return true;
	}

	// 7. 장비 장착 (올바른 equipmentInstanceId 전달)
	int enhancementLevel = 0; // TODO: DB에서 조회 필요
	EEquipItemResult equipResult = player->EquipItem(slotType, itemId, equipmentInstanceId, enhancementLevel);
	if (equipResult != EEquipItemResult::Success)
	{
		// 롤백: 새 장비를 인벤토리에 복구, 기존 장비를 다시 장착
		player->AddItem(itemId, 1, equipmentInstanceId); // 인벤토리에 다시 추가
		if (hasOldEquipment)
		{
			player->RemoveItem(returnedSlotIndex, 1, false); // Instance 보존
			player->EquipItem(slotType, oldItemId, oldEquipmentInstanceId, oldEnhancementLevel);
		}
		Protocol::S_EquipItemReply replyPkt;
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Failed to equip item");
		session->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));
		return true;
	}

	// 8. 응답
	Protocol::S_EquipItemReply replyPkt;
	replyPkt.set_success(true);
	replyPkt.set_errormessage("");
	replyPkt.set_slottype(slotType);

	// changedSlotInfo (인벤토리에서 제거된 슬롯)
	auto* changedSlot = replyPkt.mutable_changedslotinfo();
	const ItemSlot& currentSlot = player->GetInventory().GetSlot(inventorySlotIndex);
	changedSlot->set_slotindex(inventorySlotIndex);
	changedSlot->set_itemid(currentSlot.itemId);
	changedSlot->set_count(currentSlot.count);
	changedSlot->set_isquickslot(currentSlot.isQuickSlot);
	changedSlot->set_equipmentinstanceid(currentSlot.equipmentInstanceId);

	// returnedEquipmentSlot (기존 장비가 인벤토리로 돌아온 경우)
	if (hasOldEquipment && returnedSlotIndex >= 0)
	{
		auto* returnedSlot = replyPkt.mutable_returnedequipmentslot();
		const ItemSlot& retSlot = player->GetInventory().GetSlot(returnedSlotIndex);
		returnedSlot->set_slotindex(returnedSlotIndex);
		returnedSlot->set_itemid(retSlot.itemId);
		returnedSlot->set_count(retSlot.count);
		returnedSlot->set_isquickslot(retSlot.isQuickSlot);
		returnedSlot->set_equipmentinstanceid(retSlot.equipmentInstanceId);
	}

	session->Send(ClientPacketHandler::MakeSendBuffer(replyPkt));

	// 9. 룸 내 다른 플레이어에게 장비 변경 브로드캐스트
	RoomRef room = player->GetRoom();
	if (room)
	{
		room->DoAsync([room, player, slotType, itemId]()
		{
			Protocol::S_BroadcastPlayerEquipment broadcastPkt;
			broadcastPkt.set_playerid(player->playerId);
			broadcastPkt.set_slottype(slotType);
			broadcastPkt.set_itemid(itemId);

			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(broadcastPkt);
			room->Broadcast(sendBuffer);
		});
	}

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_EquipItemRequest]: Success - ItemId[%d] InstanceId[%d] equipped to SlotType[%d]\n",
		itemId, equipmentInstanceId, static_cast<int>(slotType));

	return true;
}

bool Handle_C_UnequipItemRequest(PacketSessionRef& session, Protocol::C_UnequipItemRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	Protocol::EEquipmentSlotType slotType = pkt.slottype();

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_UnequipItemRequest]: Player request to unequip item from slot[%d] \n", static_cast<int>(slotType));

	Protocol::S_UnequipItemReply replyPkt;

	// 1. Validate equipment slot is not empty
	if (player->GetEquipment().IsSlotEmpty(slotType))
	{
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Equipment slot is empty");
		auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(SendBuffer);
		return true;
	}

	// 2. Get equipped item info (itemId + equipmentInstanceId)
	const EquipmentSlot& equipSlot = player->GetEquipment().GetEquipmentSlot(slotType);
	int itemId = equipSlot.itemId;
	int equipmentInstanceId = equipSlot.equipmentInstanceId;

	if (itemId == 0)
	{
		replyPkt.set_success(false);
		replyPkt.set_errormessage("Invalid equipped item");
		auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(SendBuffer);
		return true;
	}

	// 3. Unequip item first (before adding to inventory)
	player->UnequipItem(slotType);

	// 4. Add equipment to inventory (with equipmentInstanceId)
	EAddItemResult addResult = player->AddItem(itemId, 1, equipmentInstanceId);
	if (addResult != EAddItemResult::Success)
	{
		// Rollback: re-equip the item
		player->EquipItem(slotType, itemId, equipmentInstanceId, equipSlot.enhancementLevel);

		replyPkt.set_success(false);
		replyPkt.set_errormessage("Inventory is full");
		auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
		session->Send(SendBuffer);
		return true;
	}

	// 5. Send success reply
	replyPkt.set_success(true);
	replyPkt.set_errormessage("");
	replyPkt.set_slottype(slotType);

	// Find the inventory slot where the item was added (by equipmentInstanceId)
	const InventorySystem& inventory = player->GetInventory();
	for (int i = 0; i < INVENTORY_TOTAL_SLOTS; ++i)
	{
		const ItemSlot& slot = inventory.GetSlot(i);
		if (slot.equipmentInstanceId == equipmentInstanceId && slot.itemId == itemId)
		{
			auto* changedSlot = replyPkt.mutable_returnedequipmentslot();
			changedSlot->set_slotindex(i);
			changedSlot->set_itemid(slot.itemId);
			changedSlot->set_count(slot.count);
			changedSlot->set_isquickslot(slot.isQuickSlot);
			changedSlot->set_equipmentinstanceid(slot.equipmentInstanceId);
			break;
		}
	}

	auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(SendBuffer);

	// 6. 룸 내 다른 플레이어에게 장비 해제 브로드캐스트
	RoomRef room = player->GetRoom();
	if (room)
	{
		room->DoAsync([room, player, slotType]()
		{
			Protocol::S_BroadcastPlayerEquipment broadcastPkt;
			broadcastPkt.set_playerid(player->playerId);
			broadcastPkt.set_slottype(slotType);
			broadcastPkt.set_itemid(0); // 0 = 장비 해제

			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(broadcastPkt);
			room->Broadcast(sendBuffer);
		});
	}

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_UnequipItemRequest]: Success - ItemId[%d] InstanceId[%d] unequipped from SlotType[%d]\n",
		itemId, equipmentInstanceId, static_cast<int>(slotType));

	return true;
}

bool Handle_C_EquipmentInfoRequest(PacketSessionRef& session, Protocol::C_EquipmentInfoRequest& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (gameSession->GetState() != GameSession::State::InRoom)
		return false;

	PlayerRef player = gameSession->_currentPlayer;
	if (!player)
		return false;

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[C_EquipmentInfoRequest]: Player가 장비 정보 조회 요청함 \n");

	// 플레이어의 장비 정보를 가져와서 응답 패킷 생성
	const EquipmentSystem& equipment = player->GetEquipment();

	Protocol::S_EquipmentInfoReply replyPkt;

	for (int i = 0; i < EQUIPMENT_TOTAL_SLOTS; ++i)
	{
		Protocol::EEquipmentSlotType slotType = static_cast<Protocol::EEquipmentSlotType>(i);
		const EquipmentSlot& slot = equipment.GetEquipmentSlot(slotType);

		if (!slot.IsEmpty())
		{
			auto* equipSlotInfo = replyPkt.add_equipments();
			equipSlotInfo->set_slottype(slotType);
			equipSlotInfo->set_equipmentinstanceid(slot.equipmentInstanceId);
			equipSlotInfo->set_itemid(slot.itemId);
			equipSlotInfo->set_enhancementlevel(slot.enhancementLevel);
		}
	}

	auto SendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(SendBuffer);

	return true;
}

bool Handle_C_NpcShopSellRequest(PacketSessionRef& session, Protocol::C_NpcShopSellRequest& pkt)
{
	return false;
}
