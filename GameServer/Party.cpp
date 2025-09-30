#include "pch.h"
#include "Party.h"
#include "Player.h"
#include "GameSession.h"

Party::Party(int32 partyId, PlayerRef leader)
    : _partyId(partyId), _leader(leader)
{
    _members.reserve(MAX_MEMBERS);
}

bool Party::AddMember(PlayerRef player)
{
    // PlayerRef가 Nullptr인 경우
    if (!player)
    {
        GConsoleLogger->WriteStdErr(Color::RED, L"PlayerRef is nullptr Check!");
        return false;
    }

    // PlayerRef에 파티가 이미 있는경우
    if (player->IsInParty())
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"Player is already in another party.");
        return false;
    }

    // 이미 같은 파티에 있는 경우
    if (player->GetPartyId() == _partyId)
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"Player is already in same party. partyId = %d", _partyId);
        return false;
    }

    // 방장이 자기 자신을 넣으려는 경우
    if (IsLeader(player))
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"Player is the TeamLeader, So don't get in partyId = %d", _partyId);
        return false;
    }

    _members.push_back(player);
    return true;

}

bool Party::RemoveMember(PlayerRef player)
{
    auto it = std::find(_members.begin(), _members.end(), player);
    if (it == _members.end())
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"Player is not found in partyId = %d", _partyId);
        return false;
    }

    _members.erase(it);
    return true;
}

bool Party::IsFull() const
{
    return _members.size() == MAX_MEMBERS;
}

bool Party::IsLeader(PlayerRef player) const
{
    return _leader == player;
}

int32 Party::GetPartyId() const
{
    return _partyId;
}

PlayerRef Party::GetLeader() const
{
    return _leader;
}

const Vector<PlayerRef>& Party::GetMembers() const
{
    return _members;
}

Vector<PlayerRef> Party::GetOnlineMembers()
{
    Vector<PlayerRef> onlineMembers;
    
    for (const auto& member : _members)
    {
        if (auto s = member->ownerSession.lock())
        {
            if(s->IsInGame()) onlineMembers.push_back(member);
        }
    }

    return onlineMembers;
}

const Vector<Protocol::PartyMemberInfoStatus> Party::GetMemberInfoStatus() const
{
    Vector<Protocol::PartyMemberInfoStatus> memberInfos;
    memberInfos.reserve(MAX_MEMBERS);
    
    for (const auto& member : _members)
    {
        Protocol::PartyMemberInfoStatus memberInfo;
        memberInfo.set_playerid(member->playerId);
        memberInfo.set_hp(member->Hp());
        memberInfo.set_maxhp(member->MaxHp());
        memberInfo.set_level(member->Level());
        memberInfo.set_isleader(GetLeader() == member);
        memberInfos.push_back(std::move(memberInfo));
    }

    return memberInfos;
}
