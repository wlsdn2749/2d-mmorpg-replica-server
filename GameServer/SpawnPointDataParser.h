#pragma once
#include "MonsterTypes.h"

#include "JsonDataParser.h"

/**
 * SpawnPoint_data.json 파일을 파싱하는 특화 클래스
 * Google Sheets에서 생성된 스폰 포인트 데이터를 SpawnPointCfg 구조체로 변환
 */

class SpawnPointDataParser
{
public:
	struct SpawnPointCfgRecord
	{
		SpawnPointCfg cfg;

		SpawnPointCfgRecord() = default;
		SpawnPointCfgRecord(const SpawnPointCfg& cfg)
			: cfg(cfg) { }
	};


	// Id를 Key로 하는 SpawnPointMap 로드
    static std::unordered_map<int, SpawnPointCfgRecord> LoadSpawnPointData();

	// Json Value 객체를 SpawnPointCfg 객체로 변환
    static SpawnPointCfg JsonToMSpawnPointCfg(const rapidjson::Value& json);

	// Json Value에서 SpawnPointCfgRecord 객체로 변환
    static SpawnPointCfgRecord JsonToSpawnPointCfgRecord(const rapidjson::Value& json);

private:
    static int ExtractId(const rapidjson::Value& json);
};

