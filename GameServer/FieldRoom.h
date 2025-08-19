#pragma once
#include "Room.h"
#include "MapData.h"
// 마을 룸 :
class FieldRoom : public Room
{
public:
	using Room::Room; // Room::Cfg 생성자를 상속함.

	bool CanEnterTile(const PlayerRef& p, int nx, int ny) const override {
		(void)p; // p를 안쓰겠다는 관용구
		return !_map->IsBlocked(nx, ny);
	}

	// Chat

protected:
	// 최초 1회 초기화
	void StartTick() override;

	// 입장/퇴장 훅
	void OnEnter(const PlayerRef& p) override;
	void OnLeave(const PlayerRef& p) override;

	//std::optional<SpawnPoint> ResolveSpawn(int portalId) const override; // 스폰

	// 매 틱 호출
	void OnRoomTick() override;

	// 이동 로직
	void OnPlayerMoved(const PlayerRef& p, int ox, int oy) override;
};

