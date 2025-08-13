#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

#include "AccountRepository.h"
#include "CharacterRepository.h"
#include "JwtAuth.h"

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

	Protocol::S_CharacterListReply reply;
	auto* out = reply.mutable_characters();
	out->Reserve(static_cast<int>(characters.size()));
	for (const auto& m : characters) {
		out->Add()->CopyFrom(m);
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(reply);
	session->Send(sendBuffer);
	return true;
	
}

bool Handle_C_PlayerMoveRequest(PacketSessionRef& session, Protocol::C_PlayerMoveRequest& pkt)
{
	return false;
}
