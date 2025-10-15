#pragma once
#include "Npc.h"

class NpcManager
{
#pragma region Meyers Singleton
public:
	static NpcManager& Instance()
	{
		static NpcManager npcManager;
		return npcManager;
	}

	NpcManager(const NpcManager&) = delete;
	NpcManager& operator=(const NpcManager&) = delete;

private:
	NpcManager() = default;
	~NpcManager() = default;
#pragma endregion

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

