#include "pch.h"
#include "NpcManager.h"
#include "NpcDataParser.h"

bool NpcManager::Initialize()
{
	if(_initialized)
		return true;

	GConsoleLogger->WriteStdOut(Color::YELLOW, L"NpcManager: Initializing...\n");

	_npcConfigMap = NpcDataParser::LoadNpcConfig(); // move

	_initialized = true;

	GConsoleLogger->WriteStdOut(Color::GREEN, L"NpcManager: Initialization complete. Total %d shops loaded.\n",
		static_cast<int>(_npcConfigMap.size()));

	return true;
}

void NpcManager::Shutdown()
{
	if(!_initialized)
		return ;

	_npcConfigMap.clear();

	_initialized = false;
}

const Vector<NpcConfig> NpcManager::GetNpcConfigsByMapId(int mapId) const
{
	Vector<NpcConfig> npcConfigs {};

	for (const auto& [npcId, npcConfig] : _npcConfigMap)
	{
		if (npcConfig->mapId == mapId)
		{
			npcConfigs.emplace_back(*npcConfig.get());
		}
	}

	return npcConfigs;
}
