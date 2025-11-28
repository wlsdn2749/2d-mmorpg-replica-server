#include "pch.h"
#include "Party.h"
#include "Player.h"
#include "GameSession.h"

Party::Party(int32 partyId, const string& partyName, PlayerRef leader)
    : _partyId(partyId), _partyName(partyName), _leader(leader)
{
    _members.reserve(MAX_MEMBERS);
    _members.push_back(leader);
}

void Party::SetPartyName(const string& name)
{
    _partyName = name;
}

bool Party::AddMember(PlayerRef player)
{
    // PlayerRef가 Nullptr인 경우
    if (!player)
    {
        GConsoleLogger->WriteStdErr(Color::RED, L"PlayerRef is nullptr Check!\n");
        return false;
    }

    // PlayerRef에 파티가 이미 있는경우
    if (player->IsInParty())
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"Player is already in another party.\n");
        return false;
    }

    // 이미 같은 파티에 있는 경우
    if (player->GetPartyId() == _partyId)
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"Player is already in same party. partyId = %d\n", _partyId);
        return false;
    }

    // 방장이 자기 자신을 넣으려는 경우
    if (IsLeader(player))
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"Player is the TeamLeader, So don't get in partyId = %d\n", _partyId);
        return false;
    }

    // 가득 찬경우
    if (IsFull())
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"Party Is Full\n");
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

const Vector<Protocol::PartyMemberStatusInfo> Party::GetMemberStatusInfo() const
{
    Vector<Protocol::PartyMemberStatusInfo> memberInfos;
    memberInfos.reserve(MAX_MEMBERS);
    
    for (const auto& member : _members)
    {
        Protocol::PartyMemberStatusInfo memberInfo;
        memberInfo.set_playerid(member->playerId);
		memberInfo.set_playername(member->username);
        memberInfo.set_hp(member->Hp());
        memberInfo.set_maxhp(member->MaxHp());
        memberInfo.set_level(member->Level());
        memberInfo.set_isleader(GetLeader() == member);
        memberInfos.push_back(std::move(memberInfo));
    }

    return memberInfos;
}

string Party::GetPartyName() const
{
    return _partyName;
}

int32 Party::GetCurrentMemberCount() const
{
    return _members.size();
}

Protocol::PartyInfo Party::GetPartyInfo() const
{
    Protocol::PartyInfo info;
    info.set_partyid(_partyId);
    info.set_partyname(_partyName);
    
    auto memberInfos = GetMemberStatusInfo();
    for (auto& member : memberInfos)
    {
        *info.add_members() = member;
    }

    info.set_curmembercount(GetCurrentMemberCount());
    info.set_maxmembercount(MAX_MEMBERS);
    info.set_partyleaderid(GetLeader()->playerId);

    return info;
}

bool Party::SetLeader(PlayerRef target)
{
    if(!target) return false;

	_leader = target;
}
