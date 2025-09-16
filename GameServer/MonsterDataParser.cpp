#include "pch.h"
#include "MonsterDataParser.h"
#include "ConsoleLogger.h"

std::unordered_map<int, MonsterDataParser::MonsterDataRecord> MonsterDataParser::LoadMonsterData()
{
    try
    {
        GConsoleLogger->WriteStdOut(Color::GREEN, L"몬스터 데이터 로딩 시작: Monster_data.json\n");

        auto monsterData = JsonDataParser::ParseMapData<MonsterDataRecord>(
            "Monster_data.json",
            [](const rapidjson::Value& json) { return JsonToMonsterRecord(json); },
            [](const rapidjson::Value& json) { return ExtractMonsterId(json); }
        );

        GConsoleLogger->WriteStdOut(Color::GREEN, L"몬스터 데이터 로딩 완료: %d개의 몬스터 타입\n", static_cast<int>(monsterData.size()));

        // 로드된 몬스터 정보 출력
        for (const auto& [monsterId, record] : monsterData)
        {
            GConsoleLogger->WriteStdOut(Color::WHITE,
                L"  - Monster[%d] %s: HP=%d, ATK=%d, MoveSpeed=%d, AttackRange=%d, Cooldown=%dms, AggroRange=%d\n",
                record.monsterId,
                StrToWstr(record.name).c_str(),
                record.stats.maxHp,
                record.stats.atk,
                record.stats.moveSpeedTilesPerSec,
                record.stats.attackRangeTiles,
                record.stats.attackCooldownMs,
                record.stats.aggroRangeTiles
            );
        }

        return monsterData;
    }
    catch (const std::exception& e)
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"몬스터 데이터 로딩 실패: %s\n", StrToWstr(e.what()).c_str());
        throw;
    }
}

MonsterStats MonsterDataParser::JsonToMonsterStats(const rapidjson::Value& json)
{
    MonsterStats stats;

    // JSON 필드명 → MonsterStats 필드 매핑
    stats.maxHp = JsonDataParser::SafeGetInt(json, "maxHp");
    stats.atk = JsonDataParser::SafeGetInt(json, "atk");
    stats.moveSpeedTilesPerSec = JsonDataParser::SafeGetInt(json, "moveSpeed");
    stats.attackRangeTiles = JsonDataParser::SafeGetInt(json, "attackRange");
    stats.attackCooldownMs = JsonDataParser::SafeGetInt(json, "attackCooldown");
    stats.aggroRangeTiles = JsonDataParser::SafeGetInt(json, "aggroRanges");

    return stats;
}

MonsterDataParser::MonsterDataRecord MonsterDataParser::JsonToMonsterRecord(const rapidjson::Value& json)
{
    MonsterDataRecord record;

    record.monsterId = JsonDataParser::SafeGetInt(json, "monsterId");
    record.name = JsonDataParser::SafeGetString(json, "name");
    record.stats = JsonToMonsterStats(json);

    return record;
}

int MonsterDataParser::ExtractMonsterId(const rapidjson::Value& json)
{
    return JsonDataParser::SafeGetInt(json, "monsterId");
}