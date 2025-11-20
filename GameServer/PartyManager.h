#pragma once
class PartyManager
{
#pragma region Meyers Singleton
public:
	static PartyManager& Instance()
	{
		static PartyManager instance;
		return instance;
	}

	PartyManager(const PartyManager&) = delete;
	PartyManager& operator=(const PartyManager&) = delete;
private:
	PartyManager() = default;
	~PartyManager() = default;

#pragma endregion

public:
	bool Initialize();
	void Shutdown();

	// 파티 관리
	PartyRef CreateParty(PlayerRef leader);
	PartyRef CreatePartyWithName(PlayerRef leader, const string& partyName);
	bool DisbandParty(int32 partyId);
	bool JoinParty(int32 partyId, PlayerRef player);
	bool LeaveParty(PlayerRef player);
	bool kickMember(int32 partyId, PlayerRef kicker, PlayerRef target);
	bool DelegatePartyLeader(int32 partyId, PlayerRef player, PlayerRef target);

	// 조회
	PartyRef FindParty(int32 partyId);
	PartyRef FindPlayerParty(PlayerRef player);
	int32 GetPlayerPartyId(PlayerRef player);
	Vector<Protocol::PartyInfo> GetAllPublicParties(); // 전체 파티 목록 조회

	template <typename T, typename... Ts>
	bool IsSameParty(const T& first, const Ts&... rest)
	{
		return ((first == rest) && ...);
	}

	// 파티 가입 요청 관리
	bool AddJoinRequest(int32 partyId, PlayerRef requester);
	bool HasPendingRequest(int32 partyId, PlayerRef requester);
	bool RemoveJoinRequest(int32 partyId, PlayerRef requester);

	Vector<PlayerRef> GetJoinRequesters(int32 partyId); // 전체 요청자 리스트
	PlayerRef FindRequesterById(int32 partyId, int32 requesterPid); // 특정 요청자

private:
	atomic<int32> _nextPartyId {1};
	unordered_map<int32, PartyRef> _parties;
	unordered_map<PlayerRef, int32> _playerToParty;
	unordered_map<int32, Vector<PlayerRef>> _partyJoinRequests; // 파티별 가입 요청 대기엽

	USE_LOCK;
	bool _initialized = false;

};

