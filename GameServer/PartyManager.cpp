#include "pch.h"
#include "PartyManager.h"
#include "Player.h"
#include "Party.h"
#include "GameSession.h"

#include "PartyService.h"
PartyRef PartyManager::CreateParty(PlayerRef leader)
{
    if(leader->IsInParty()) return FindPlayerParty(leader);

    int32 partyId = _nextPartyId.fetch_add(1);

    PartyRef partyRef = MakeShared<Party>(partyId, leader);

    _parties[partyId] = partyRef;
    _playerToParty[leader] = partyId;


    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_JOIN);
    return partyRef;
}

bool PartyManager::DisbandParty(int32 partyId)
{
    if(!FindParty(partyId)) return false;

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
   
    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_DISBANDED);
    return true;
    
}

bool PartyManager::JoinParty(int32 partyId, PlayerRef player)
{
    if(!FindParty(partyId)) return false; // 파티 Id가 없는 경우
    if(player->IsInParty()) return false; // 이미 파티에 있는경우

    _playerToParty[player] = partyId; // 입장 
    player->SetPartyId(partyId);

    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_JOIN);
}

bool PartyManager::LeaveParty(PlayerRef player)
{
    if(!player->IsInParty()) return false; // 파티가 없는 경우

    auto partyId = player->GetPartyId();

    if (FindPlayerParty(player)->GetLeader() == player) // 리더인 경우
    {
        return DisbandParty(GetPlayerPartyId(player));
    }

    // 일반 사람인 경우
    _playerToParty.erase(player);
    player->SetPartyId(0);

    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_LEAVE);
}

bool PartyManager::kickMember(int32 partyId, PlayerRef kicker, PlayerRef target)
{
    if (!FindParty(partyId)) return false; // 파티 Id가 없는 경우
    
    if (FindPlayerParty(kicker)->GetLeader() != kicker) return false; // 방장만 강퇴 가능

    if (!IsSameParty(kicker, target)) return false; // 같은 파티가 아닐경우 못함

    _playerToParty.erase(target);
    target->SetPartyId(0);

    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_LEAVE);
    return true;
}

PartyRef PartyManager::FindParty(int32 partyId)
{
    return _parties[partyId];
}

PartyRef PartyManager::FindPlayerParty(PlayerRef player)
{
    return _parties[GetPlayerPartyId(player)];
}

int32 PartyManager::GetPlayerPartyId(PlayerRef player)
{
    return player->GetPartyId();
}