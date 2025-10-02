#include "pch.h"
#include "DropDataParser.h"

std::unordered_map<int, DropItemInfo> DropDataParser::LoadDropItemData()
{
    try
    {
        GConsoleLogger->WriteStdOut(Color::GREEN, L"드랍 데이터 로딩 시작: Drop_data.json\n");

        auto dropDatas = JsonDataParser::ParseMapData<DropItemInfo>(
            "Drop_data.json",
            [](const rapidjson::Value& json) { return JsonToDropItemInfo(json); },
            [](const rapidjson::Value& json) { return ExtractId(json); }
        );

        GConsoleLogger->WriteStdOut(Color::GREEN, L"드랍 데이터 로딩 완료: %d개의 드랍 개수\n", static_cast<int>(dropDatas.size()));

        // 로드된 아이템 정보 출력
        for (const auto& [Id, data] : dropDatas)
        {
            GConsoleLogger->WriteStdOut(Color::WHITE,
                L"  - Drop[%d] %d: ItemId=%d, DropRate=%d, MinCount=%d, MaxCount=%d IsEnabled=%s\n",
                data.id,
                data.monsterId,
                data.itemId,
                data.dropRate,
                data.minCount,
                data.maxCount,
                data.isEnabled ? L"True" : L"False"
            );
        }

        return dropDatas;
    }
    catch (const std::exception& e)
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"드랍 데이터 로딩 실패: %s\n", StrToWstr(e.what()).c_str());
        throw;
    }
}

DropItemInfo DropDataParser::JsonToDropItemInfo(const rapidjson::Value& json)
{
    DropItemInfo dropItemInfo;

    // Json 필드명 -> ItemData 필드 매핑
    dropItemInfo.id = JsonDataParser::SafeGetInt(json, "id");
    dropItemInfo.monsterId = JsonDataParser::SafeGetInt(json, "monsterId");
    dropItemInfo.itemId = JsonDataParser::SafeGetInt(json, "itemId");
    dropItemInfo.dropRate = JsonDataParser::SafeGetInt(json, "dropRate");
    dropItemInfo.minCount = JsonDataParser::SafeGetInt(json, "minCount");
    dropItemInfo.maxCount = JsonDataParser::SafeGetInt(json, "maxCount");
    dropItemInfo.isEnabled = JsonDataParser::SafeGetBool(json, "isEnabled");

    return dropItemInfo;
}

int DropDataParser::ExtractId(const rapidjson::Value& json)
{
    return JsonDataParser::SafeGetInt(json, "id");
}
