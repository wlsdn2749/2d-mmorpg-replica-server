#pragma once

#include "Singleton.h"
#include "Npc.h"

class NpcManager : public Singleton<NpcManager>
{
public:
	// 초기화 및 정리
	bool Initialize();
	void Shutdown();

public:
	// 특정 mapId를 가진 모든 Npc 조회
	const Vector<NpcConfig> GetNpcConfigsByMapId(int mapId) const;

private:
	// npcId -> npcConfig
	std::unordered_map<int, std::unique_ptr<NpcConfig>> _npcConfigMap;
	bool _initialized = false;

};