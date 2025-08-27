#pragma once
#include "Room.h"
#include "MapData.h"

#include "MonsterPorts.h"
#include "MonsterTypes.h"
#include "MonsterContainer.h"
#include "MonsterSpawnerSystem.h"
#include "MonsterMovementSystem.h"
#include "MonsterCombatSystem.h"
#include "MonsterService.h"

#include "PlayerPorts.h"
#include "PlayerCombatSystem.h"


// 마을 룸 :
class FieldRoom : public Room
{
public:
    FieldRoom(Room::Cfg cfg, std::shared_ptr<MapData> map)
        : Room(std::move(cfg), std::move(map))
        , _mapQuery(*this)
        , _linker(*this)
        , _cast(*this)
        , _pLinker(*this)
        , _pCaster(*this)
    {
    }

	bool CanEnterTile(int nx, int ny) const override {
		return !_map->IsBlocked(nx, ny);
	}

protected:
	// 최초 1회 초기화
	void StartTick() override;

	// 입장/퇴장 훅
	void OnEnter(const PlayerRef& p) override;
	void OnLeave(const PlayerRef& p) override;

    // 플레이어 처리 훅
    void OnPlayerHpChanged(int playerId, int newHp) override;
    void OnPlayerDeath(int playerId, int killerMonsterId) override;

	//std::optional<SpawnPoint> ResolveSpawn(int portalId) const override; // 스폰

	// 매 틱 호출
	void OnRoomTick() override;

	// 이동 로직
	void OnPlayerMoved(const PlayerRef& p, int ox, int oy) override;
    
    // 공격 로직
    void OnRecvAttackReq(const PlayerRef& p, const Protocol::C_PlayerAttackRequest& req) override;

private:
/*--------------------------------
        몬스터 어댑터
--------------------------------*/
    class MonsterMapQueryImpl : public IMonsterMapQuery {
    public:
        explicit MonsterMapQueryImpl(const FieldRoom& r) : _r(r) {}
        bool CanEnter(int x, int y) const override { return _r.CanEnterTile(x, y); }
    private: 
        const FieldRoom& _r;
    };

    class MonsterEntityLinkerImpl : public IMonsterEntityLinker {
    public:
        explicit MonsterEntityLinkerImpl(FieldRoom& r) : _r(r) {}
        void ForEachPlayerInRange(int cx, int cy, int rangeTiles,
            std::function<void(const PlayerView&)>) const override;
        bool TryGetPlayer(int playerId, PlayerView& out) const override;
        void ApplyDamageToPlayer(int playerId, int damage, int sourceMonsterId) override;
    private: 
        FieldRoom& _r;
    };

    class MonsterBroadcasterImpl : public IMonsterBroadcaster {
    public:
        explicit MonsterBroadcasterImpl(FieldRoom& r) : _r(r) {}
        void SpawnMonster(const Monster& m) override;
        void DespawnMonster(EntityId id, Protocol::EDespawnReason reason) override;
        void BroadcastMonsterMove(EntityId id, int x, int y, Protocol::EDirection dir) override;
        void BroadcastMonsterAttack(EntityId id, int targetPid) override;
        void BroadcastMonsterDeath(EntityId id) override;
    private: 
        FieldRoom& _r;
    };

    class MonsterClockImpl : public IMonsterClock {
    public:
        int64_t NowMs() const override; // TODO: 서버 공용 시계 사용해도 OK
    };

    class MonsterRngImpl : public IMonsterRng {
    public:
        int NextInt(int minIncl, int maxIncl) override;
    };

/*--------------------------------
        플레이어 어댑터
--------------------------------*/
    class PlayerMonsterLinkerImpl : public IPlayerMonsterLinker {
    public:
        explicit PlayerMonsterLinkerImpl(FieldRoom& r) : _r(r) {}
	    virtual void ForEachMonsterInRange(int cx, int cy, int rangeTiles, std::function<void(const MonsterView&)>) const override; 
	    virtual bool TryGetMonster(EntityId monsterId, MonsterView& outMv) const override;
	    virtual bool ApplyDamageToMonster(int monsterId, int damage, int srcPlayerId) override;
    private:
        FieldRoom& _r;
    };

    class PlayerCombatBroadcasterImpl : public IPlayerCombatBroadcaster {
    public:
        explicit PlayerCombatBroadcasterImpl(FieldRoom& r) : _r(r) {}
        virtual void BroadcastPlayerAttack(int attackerId, int targetId, int damage, int hpAfter) override;
    private:
        FieldRoom& _r;
    };

private:
    // ==== 어댑터 인스턴스 & 몬스터 서비스 ====
    MonsterMapQueryImpl     _mapQuery;
    MonsterEntityLinkerImpl _linker;
    MonsterBroadcasterImpl  _cast;
    MonsterClockImpl        _clock;
    MonsterRngImpl          _rng;

    std::unique_ptr<MonsterService> _monsters;

    void InitMonsters(); // 구성/초기 스폰
    int64_t _lastMonsterTickMs{ 0 }; // monster Dt 계산용


    // ==== 플레이어 어댑터 ====
    PlayerMonsterLinkerImpl _pLinker;
    PlayerCombatBroadcasterImpl _pCaster;

    std::unique_ptr<PlayerCombatSystem> _pCombat;

    void InitPCombat();

};

