#pragma once
#include "PartyManager.h"
#include "Player.h"
#include "Party.h"
#include "Room.h"

class PartyService {
#pragma region Meyers Singleton
public:
    static PartyService& Instance() {
        static PartyService instance;
        return instance;
    }

    PartyService(const PartyService&) = delete;
    PartyService& operator=(const PartyService&) = delete;
private:
    PartyService() = default;
    ~PartyService() = default;

#pragma endregion

public:
    bool Initialize();
    void Shutdown();

    // 경험치 분배 (MonsterCombatSystem에서 호출)
    void DistributeExp(PlayerRef killer, int32 baseExp);

    // 파티 상태 동기화 (Room에서 주기적 호출)
    void UpdatePartyStatuses(const vector<PlayerRef>& roomPlayers, const Protocol::EPartyUpdateType& updateType = Protocol::EPartyUpdateType::PARTY_UPDATE_STATUS);

    // 파티 상태 업데이트 송신
    void SendPartyStatusUpdate(int32 partyId, const Protocol::EPartyUpdateType& updateType);

private:
    void BroadcastToPartyMembers(PartyRef party, Protocol::S_BroadcastPartyUpdate& pkt);

private:
    bool _initialized = false;
};
