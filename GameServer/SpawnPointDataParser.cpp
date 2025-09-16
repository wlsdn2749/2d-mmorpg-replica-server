#include "pch.h"
#include "SpawnPointDataParser.h"

std::unordered_map<int, SpawnPointDataParser::SpawnPointCfgRecord> SpawnPointDataParser::LoadSpawnPointData()
{
    try
    {
        GConsoleLogger->WriteStdOut(Color::GREEN, L"스폰 포인트 데이터 로딩 시작: SpawnPoint_data.json\n");

        auto spawnPointData = JsonDataParser::ParseMapData<SpawnPointCfgRecord>(
            "SpawnPoint_data.json",
            [](const rapidjson::Value& json) { return JsonToSpawnPointCfgRecord(json); },
            [](const rapidjson::Value& json) { return ExtractId(json); }
        );

        GConsoleLogger->WriteStdOut(Color::GREEN, L"스폰 포인트 데이터 로딩 완료: %d개의 스폰포인트\n", static_cast<int>(spawnPointData.size()));

        // 로드된 스폰 포인트 정보 출력
        for (const auto& [monsterId, spawnPoint] : spawnPointData)
        {
            GConsoleLogger->WriteStdOut(Color::WHITE,
                L"  - SpawnPoint[%d]: X=%d, Y=%d, MaxAlive=%d, initialSpawn=%d, respawnDelayMs=%dms, leashRadiusTiles=%d, monsterTypeId=%d\n",
                spawnPoint.cfg.id,
                spawnPoint.cfg.x,
                spawnPoint.cfg.y,
                spawnPoint.cfg.maxAlive,
                spawnPoint.cfg.initialSpawn,
                spawnPoint.cfg.respawnDelayMs,
                spawnPoint.cfg.leashRadiusTiles,
                spawnPoint.cfg.monsterTypeId
            );
        }

        return spawnPointData;
    }
    catch (const std::exception& e)
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"몬스터 데이터 로딩 실패: %s\n", StrToWstr(e.what()).c_str());
        throw;
    }
}

SpawnPointCfg SpawnPointDataParser::JsonToMSpawnPointCfg(const rapidjson::Value& json)
{
    SpawnPointCfg cfg;

    // JSON 필드명 → MonsterStats 필드 매핑
    cfg.id               = JsonDataParser::SafeGetInt(json, "id");
    cfg.x                = JsonDataParser::SafeGetInt(json, "x");
    cfg.y                = JsonDataParser::SafeGetInt(json, "y");
    cfg.maxAlive         = JsonDataParser::SafeGetInt(json, "maxAlive");
    cfg.initialSpawn     = JsonDataParser::SafeGetInt(json, "initialSpawn");
    cfg.respawnDelayMs   = JsonDataParser::SafeGetInt(json, "respawnDelayMs");
    cfg.leashRadiusTiles = JsonDataParser::SafeGetInt(json, "leashRadiusTiles");
    cfg.monsterTypeId    = JsonDataParser::SafeGetInt(json, "monsterTypeId");

    return cfg;
}

SpawnPointDataParser::SpawnPointCfgRecord SpawnPointDataParser::JsonToSpawnPointCfgRecord(const rapidjson::Value& json)
{
    SpawnPointCfgRecord record;

    record.cfg = JsonToMSpawnPointCfg(json);

    return record;
}

int SpawnPointDataParser::ExtractId(const rapidjson::Value& json)
{
    return JsonDataParser::SafeGetInt(json, "id");
}
