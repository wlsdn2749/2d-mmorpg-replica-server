#include "pch.h"
#include "PartyService.h"

#include "ClientPacketHandler.h"


void PartyService::DistributeExp(PlayerRef killer, int32 baseExp)
{
	auto party = PartyManager::Instance().FindPlayerParty(killer);

	if (!party)
	{
		killer->AddExp(baseExp);
		return;
	}

	auto onlineMembers = party->GetOnlineMembers();

	// 처치자: 110% 경험치
	killer->AddExp(static_cast<int32>(baseExp * 1.1f));

	// 나머지 파티원: 각각 10% 경험치 주기 
	for (auto& member : onlineMembers)
	{
		if (member != killer)
		{
			member->AddExp(static_cast<int32>(baseExp * 0.1f));
		}
	}
}

void PartyService::UpdatePartyStatuses(const vector<PlayerRef>& roomPlayers, const Protocol::EPartyUpdateType& updateType)
{
	unordered_map<int32, vector<PlayerRef>> partiesInRoom;

	// 파티 별로 그룹화
	for (auto& player : roomPlayers)
	{
		int32 partyId = player->GetPartyId();
		partiesInRoom[partyId].push_back(player);
	}

	for (auto& [partyId, members] : partiesInRoom)
	{
		SendPartyStatusUpdate(partyId, updateType);
	}
}

void PartyService::SendPartyStatusUpdate(int32 partyId, const Protocol::EPartyUpdateType& updateType) {
	auto party = PartyManager::Instance().FindParty(partyId);
	if (!party) return;

	// S_BroadcastPartyUpdate 패킷 생성
	Protocol::S_BroadcastPartyUpdate pkt;
	pkt.set_updatetype(updateType);

	// 파티 전체 멤버의 상태 정보 수집
	for (auto& member : party->GetOnlineMembers()) {
		auto* memberInfo = pkt.add_members();
		memberInfo->set_playerid(static_cast<int32>(member->playerId));
		memberInfo->set_hp(member->Hp());
		memberInfo->set_maxhp(member->MaxHp());
		memberInfo->set_level(member->Level());
		memberInfo->set_isleader(party->IsLeader(member));
	}

	// 파티원들에게 브로드캐스트
	BroadcastToPartyMembers(party, pkt);
}


void PartyService::BroadcastToPartyMembers(PartyRef party, Protocol::S_BroadcastPartyUpdate& pkt)
{
	for (const auto& player : party->GetOnlineMembers())
	{
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
		if(auto s = player->ownerSession.lock())
			s->Send(sendBuffer);
	}
}

