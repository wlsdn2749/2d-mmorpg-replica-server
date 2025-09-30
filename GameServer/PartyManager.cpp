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

    PartyRef partyRef = MakeShared<Party>(partyId, leader);

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

    if (!IsSameParty(kicker, target)) return false; // 같은 파티가 아닐경우 못함

    party->RemoveMember(target);

    _playerToParty.erase(target);
    target->SetPartyId(0);

    PartyService::Instance().SendPartyStatusUpdate(partyId, Protocol::EPartyUpdateType::PARTY_UPDATE_MEMBER_LEAVE);
    return true;
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