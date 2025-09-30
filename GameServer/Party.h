#pragma once
class Party
{
public:
	bool operator==(const Party& rhs){return this->_partyId == rhs.GetPartyId(); }
	bool operator!=(const Party& rhs){return this->_partyId != rhs.GetPartyId(); }
public:
	static constexpr int32 MAX_MEMBERS = 4;

	explicit Party(int32 partyId, PlayerRef leader);

	// 멤버 관리
	bool AddMember(PlayerRef player);
	bool RemoveMember(PlayerRef player);
	bool IsFull() const;
	bool IsLeader(PlayerRef player) const;

	// 조회
	int32 GetPartyId() const;
	PlayerRef GetLeader() const;
	const Vector<PlayerRef>& GetMembers() const;
	Vector<PlayerRef> GetOnlineMembers();
	const Vector<Protocol::PartyMemberInfoStatus> GetMemberInfoStatus() const;
private:
	int32 _partyId;
	PlayerRef _leader;
	Vector<PlayerRef> _members;
};

