#include "pch.h"
#include "PartyManager.h"
#include "Player.h"
#include "Party.h"
#include "GameSession.h"

#include "PartyService.h"
PartyRef PartyManager::CreateParty(PlayerRef leader)
{
    WRITE_LOCK;

    if(leader->IsInParty()) return FindPlayerParty(leader);

    int32 partyId = _nextPartyId.fetch_add(1);

    PartyRef partyRef = MakeShared<Party>(partyId, "", leader);

    _parties[partyId] = partyRef;
    _playerToParty[leader] = partyId;

    leader->SetPartyId(partyId);

    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_JOIN);
    return partyRef;
}

PartyRef PartyManager::CreatePartyWithName(PlayerRef leader, const string& partyName)
{
    WRITE_LOCK;

    if (leader->IsInParty()) return FindPlayerParty(leader);

    int32 partyId = _nextPartyId.fetch_add(1);

    PartyRef partyRef = MakeShared<Party>(partyId, partyName, leader);

    _parties[partyId] = partyRef;
    _playerToParty[leader] = partyId;

    leader->SetPartyId(partyId);

    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_JOIN);
    return partyRef;
}

bool PartyManager::DisbandParty(int32 partyId)
{
    WRITE_LOCK;

    if(!FindParty(partyId)) return false;

	// 먼저 보내야, 정보가 살아있음
	PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_DISBANDED);

    for (auto it = _playerToParty.begin(); it != _playerToParty.end();)
    {
        if (it->second == partyId)
        {
            it->first->SetPartyId(0); // 해소
            it = _playerToParty.erase(it);
        }
        else
        {
            ++it;
        }
    }

    _parties.erase(partyId);
   
  
    return true;
    
}

bool PartyManager::JoinParty(int32 partyId, PlayerRef player)
{
    WRITE_LOCK;
    auto party = FindParty(partyId);
    if(!party) return false; // 파티 Id가 없는 경우
    if(player->IsInParty()) return false; // 이미 파티에 있는경우

    party->AddMember(player);

    _playerToParty[player] = partyId; // 입장 
    player->SetPartyId(partyId);


    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_JOIN);

    return true;
}

bool PartyManager::LeaveParty(PlayerRef player)
{
    WRITE_LOCK;

    if(!player->IsInParty()) return false; // 파티가 없는 경우

    auto partyId = player->GetPartyId();
    auto party = FindParty(partyId);
    
    if(!party) return false;

    if (party->GetLeader() == player) // 리더인 경우
    {
        return DisbandParty(GetPlayerPartyId(player));
    }

    party->RemoveMember(player);

    // 일반 사람인 경우
    _playerToParty.erase(player);
    player->SetPartyId(0);


    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_LEAVE);
    return true;
}

bool PartyManager::kickMember(int32 partyId, PlayerRef kicker, PlayerRef target)
{
    WRITE_LOCK;

    auto party = FindParty(partyId);

    if (!party) return false; // 파티 Id가 없는 경우
    
    if (party->GetLeader() != kicker) return false; // 방장만 강퇴 가능

    if (!IsSameParty(kicker->GetPartyId(), target->GetPartyId())) return false; // 같은 파티가 아닐경우 못함

    party->RemoveMember(target);

    _playerToParty.erase(target);
    target->SetPartyId(0);

    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_LEAVE);
    return true;
}

bool PartyManager::DelegatePartyLeader(int32 partyId, PlayerRef player, PlayerRef target)
{
	auto party = FindParty(partyId);

	if(!party) return false; // 파티 Id가 없는 경우

	if(party->IsLeader(player) == false) return false; // player가 리더가 아닌경우

	if(!player) return false;
	if(!target) return false;  // 파티장과 대상이 존재하지 않는 경우

	auto result = party->SetLeader(target);

	PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_DELEGATE_LEADER);

	return result;
}

PartyRef PartyManager::FindParty(int32 partyId)
{
    READ_LOCK;
    auto it = _parties.find(partyId);
    return (it != _parties.end()) ? it->second : nullptr;
}

PartyRef PartyManager::FindPlayerParty(PlayerRef player)
{
    READ_LOCK;  
    auto partyId = player->GetPartyId();
    auto it = _parties.find(partyId);
    return (it != _parties.end()) ? it->second : nullptr;
}

int32 PartyManager::GetPlayerPartyId(PlayerRef player)
{
    READ_LOCK;
    return player->GetPartyId();
}

Vector<Protocol::PartyInfo> PartyManager::GetAllPublicParties()
{
    READ_LOCK;

    Vector<Protocol::PartyInfo> partyInfos;
    partyInfos.reserve(_parties.size());

    for (const auto& [partyId, party] : _parties)
    {
        if (!party) continue;

        partyInfos.push_back(party->GetPartyInfo());

        // 파티명이 있는 공개 파티만 조회 (파티명 없으면 초대전용 파티)
        /*if (!party->GetPartyName().empty())
        {
            partyInfos.push_back(party->GetPartyInfo());
        }*/
    }

    return partyInfos;
}

bool PartyManager::AddJoinRequest(int32 partyId, PlayerRef requester)
{
    WRITE_LOCK;

    if (!requester) return false;

    auto& requestQueue = _partyJoinRequests[partyId];

    // 이미 요청이 있는지 확인
    auto it = std::find(requestQueue.begin(), requestQueue.end(), requester);
    if (it != requestQueue.end())
    {
        return false; // 중복 요청
    }

    requestQueue.push_back(requester);
    return true;
}

bool PartyManager::HasPendingRequest(int32 partyId, PlayerRef requester)
{
    READ_LOCK;

    auto it = _partyJoinRequests.find(partyId);
    if (it == _partyJoinRequests.end()) return false;

    const auto& requestQueue = it->second;
    return std::find(requestQueue.begin(), requestQueue.end(), requester) != requestQueue.end();
}

bool PartyManager::RemoveJoinRequest(int32 partyId, PlayerRef requester)
{
    WRITE_LOCK;

    auto it = _partyJoinRequests.find(partyId);
    if (it == _partyJoinRequests.end() || it->second.empty())
    {
        return false;
    }

    // 특정 requester 찾아서 제거
    auto& requestQueue = it->second;
    auto reqIt = std::find(requestQueue.begin(), requestQueue.end(), requester);

    if (reqIt == requestQueue.end())
    {
        return false; // 해당 requester 없음
    }

    requestQueue.erase(reqIt);

    // 큐가 비었으면 맵에서도 제거
    if (requestQueue.empty())
    {
        _partyJoinRequests.erase(it);
    }

    return true;
}

PlayerRef PartyManager::FindRequesterById(int32 partyId, int32 requesterPid)
{
    READ_LOCK;

    auto it = _partyJoinRequests.find(partyId);
    if (it == _partyJoinRequests.end())
    {
        return nullptr;
    }

    const auto& requesters = it->second;
    for (const auto& requester : requesters)
    {
        if (requester->playerId == requesterPid)
        {
            return requester;
        }
    }

    return nullptr;
}

Vector<PlayerRef> PartyManager::GetJoinRequesters(int32 partyId)
{
    READ_LOCK;

    auto it = _partyJoinRequests.find(partyId);
    if (it == _partyJoinRequests.end())
    {
        return Vector<PlayerRef>(); // 빈 벡터 반환
    }

    return it->second; // 전체 요청자 리스트 반환
}
