#pragma once

#include "JsonDataParser.h"
#include "MonsterTypes.h"
#include <string>
#include <unordered_map>

/**
 * Monster_data.json 파일을 파싱하는 특화 클래스
 * Google Sheets에서 생성된 몬스터 데이터를 MonsterStats 구조체로 변환
 */
class MonsterDataParser
{
public:
    /**
     * 몬스터 데이터 레코드 (ID, 이름, 스탯 포함)
     */
    struct MonsterDataRecord
    {
        int monsterId;
        std::string name;
        MonsterStats stats;

        MonsterDataRecord() = default;
        MonsterDataRecord(int id, const std::string& n, const MonsterStats& s)
            : monsterId(id), name(n), stats(s) {}
    };

    /**
     * Monster_data.json 파일에서 모든 몬스터 데이터를 로드
     * @return monsterId를 키로 한 몬스터 데이터 맵
     * @throws std::runtime_error 파일 로드 또는 파싱 실패시
     */
    static std::unordered_map<int, MonsterDataRecord> LoadMonsterData();

    /**
     * JSON Value에서 MonsterStats 구조체로 변환
     * @param json JSON Value 객체
     * @return 변환된 MonsterStats 구조체
     */
    static MonsterStats JsonToMonsterStats(const rapidjson::Value& json);

    /**
     * JSON Value에서 완전한 MonsterDataRecord로 변환
     * @param json JSON Value 객체
     * @return 변환된 MonsterDataRecord 구조체
     */
    static MonsterDataRecord JsonToMonsterRecord(const rapidjson::Value& json);

private:
    /**
     * JSON Value에서 monsterId를 추출하는 키 추출 함수
     * @param json JSON Value 객체
     * @return 몬스터 ID
     */
    static int ExtractMonsterId(const rapidjson::Value& json);
};