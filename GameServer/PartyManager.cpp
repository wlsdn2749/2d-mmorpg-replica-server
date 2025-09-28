#include "pch.h"
#include "PartyManager.h"
#include "Player.h"
#include "Party.h"

PartyRef PartyManager::CreateParty(PlayerRef leader)
{
    int32 partyId = _nextPartyId.fetch_add(1);

    PartyRef partyRef = MakeShared<Party>(partyId, leader);

    _parties[partyId] = partyRef;
    _playerToParty[leader] = partyId;

    // 여기에도 파티연산 + Broadcast?
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

    // TODO Party 연산 + Broadcast
    return true;
    
}

bool PartyManager::JoinParty(int32 partyId, PlayerRef player)
{
    if(!FindParty(partyId)) return false; // 파티 Id가 없는 경우
    if(player->IsInParty()) return false; // 이미 파티에 있는경우

    _playerToParty[player] = partyId; // 입장 
    
    // TODO send Broadcast?

    // TODO Party에 대한 연산
}

bool PartyManager::LeaveParty(PlayerRef player)
{
    if(!player->IsInParty()) return false; // 파티가 없는 경우

    if (FindPlayerParty(player)->GetLeader() == player) // 리더인 경우
    {
        return DisbandParty(GetPlayerPartyId(player));
    }

    // 일반 사람인 경우
    _playerToParty.erase(player);

    // 여기서도 TODO send?

    // TODO Party에 대한 연산
}

bool PartyManager::kickMember(int32 partyId, PlayerRef kicker, PlayerRef target)
{
    if (!FindParty(partyId)) return false; // 파티 Id가 없는 경우
    
    if (FindPlayerParty(kicker)->GetLeader() != kicker) return false; // 방장만 강퇴 가능

    if (!IsSameParty(kicker, target)) return false; // 같은 파티가 아닐경우 못함

    _playerToParty.erase(target);

    // TODO Party에 대한 연산 + broadcast?

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
