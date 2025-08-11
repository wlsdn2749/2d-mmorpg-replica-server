#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

#include "AccountRepository.h"
#include "JwtAuth.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return true;
}

bool Handle_C_JWT_LOGIN_REQUEST(PacketSessionRef& session, Protocol::C_JWT_LOGIN_REQUEST& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// 현재 상태가 Connected, 즉 연결되기 전이면 말이안되는거.
	if (gameSession->GetState() != GameSession::State::Connected) {
		// 서버 에러 반환
	}

	auto [eLoginResult, userId] = JwtAuth::Verify(pkt.accesstoken());
	
	if (eLoginResult == Protocol::ELoginResult::SUCCESS)
	{
		// TODO: 세션에 jwt의 데이터 저장?

		// TODO: Accounts에 계정 정보 저장
		auto fut = AccountRepository::UpsertAccountAsync(userId);
	}

	Protocol::S_JWT_LOGIN_REPLY replyPkt;
	replyPkt.set_result(eLoginResult);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);
	session->Send(sendBuffer);
	return true;
}

bool Handle_C_CREATE_CHARACTER_REQUEST(PacketSessionRef& session, Protocol::C_CREATE_CHARACTER_REQUEST& pkt)
{
	return true;
}
