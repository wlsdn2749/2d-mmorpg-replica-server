#pragma once
class PartyManager
{
public:

	static PartyManager& Instance()
	{
		static PartyManager instance;
		return instance;
	}

	bool Initialize();
	void Shutdown();

	// 파티 관리
	PartyRef CreateParty(PlayerRef leader);
	bool DisbandParty(int32 partyId);
	bool JoinParty(int32 partyId, PlayerRef player);
	bool LeaveParty(PlayerRef player);
	bool kickMember(int32 partyId, PlayerRef kicker, PlayerRef target);

	// 조회
	PartyRef FindParty(int32 partyId);
	PartyRef FindPlayerParty(PlayerRef player);
	int32 GetPlayerPartyId(PlayerRef player);

	template <typename T, typename... Ts>
	bool IsSameParty(const T& first, const Ts&... rest)
	{
		return ((first == rest) && ...);
	}

private:
	PartyManager() = default;
	~PartyManager() = default;
	PartyManager(const PartyManager&) = delete;
	PartyManager& operator=(const PartyManager&) = delete;

	atomic<int32> _nextPartyId {1};
	unordered_map<int32, PartyRef> _parties;
	unordered_map<PlayerRef, int32> _playerToParty;
	USE_LOCK;
	bool _initialized = false;

};

