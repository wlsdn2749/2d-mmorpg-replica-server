#pragma once
class Party
{
public:
	bool operator==(const Party& rhs){return this->_partyId == rhs.GetPartyId(); }
	bool operator!=(const Party& rhs){return this->_partyId != rhs.GetPartyId(); }
public:
	static constexpr int32 MAX_MEMBERS = 4;

	explicit Party(int32 partyId, const string& partyName, PlayerRef leader);

	// 파티 관리
	void SetPartyName(const string& name);

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
	const Vector<Protocol::PartyMemberStatusInfo> GetMemberStatusInfo() const;
	string GetPartyName() const;
	int32 GetCurrentMemberCount() const;
	Protocol::PartyInfo GetPartyInfo() const;

	bool SetLeader(PlayerRef target);
private:
	int32 _partyId;
	PlayerRef _leader;
	string _partyName;
	Vector<PlayerRef> _members;
};

