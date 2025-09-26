#pragma once
class PartyService {
public:
    static PartyService& Instance() {
        static PartyService instance;
        return instance;
    }

    bool Initialize();
    void Shutdown();

    // 경험치 분배 (MonsterCombatSystem에서 호출)
    void DistributeExp(PlayerRef killer, int32 baseExp);

    // 파티 상태 동기화 (Room에서 주기적 호출)
    void UpdatePartyStatuses(const vector<PlayerRef>& roomPlayers);

private:
    PartyService() = default;
    ~PartyService() = default;
    PartyService(const PartyService&) = delete;
    PartyService& operator=(const PartyService&) = delete;

    bool _initialized = false;
};
