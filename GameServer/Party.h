#pragma once
class Party
{
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

private:
	int32 _partyId;
	PlayerRef _leader;
	Vector<PlayerRef> _members;
};

