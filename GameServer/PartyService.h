#pragma once
#include "Singleton.h"
#include "PartyManager.h"
#include "Player.h"
#include "Party.h"
#include "Room.h"

class PartyService : public Singleton<PartyService>
{

public:
    bool Initialize();
    void Shutdown();

    // 경험치 분배 (MonsterCombatSystem에서 호출)
    void DistributeExp(PlayerRef killer, int32 baseExp);

    // 파티 상태 동기화 (Room에서 주기적 호출)
    void UpdatePartyStatuses(const vector<PlayerRef>& roomPlayers, const Protocol::EPartyUpdateType& updateType = Protocol::EPartyUpdateType::PARTY_UPDATE_STATUS);

    // 파티 상태 업데이트 송신
    void SendPartyStatusUpdate(int32 partyId, const Protocol::EPartyUpdateType& updateType);

	// Kicked 당한 사람에게 메세지 전달
	void SendMessageToKickedPlayer(int32 partyId, PlayerRef kicker, PlayerRef target);

private:
    void BroadcastToPartyMembers(PartyRef party, Protocol::S_BroadcastPartyUpdate& pkt);

private:
    bool _initialized = false;
};
