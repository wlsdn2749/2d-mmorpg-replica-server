#include "pch.h"
#include "ItemDataParser.h"

std::unordered_map<int, ItemData> ItemDataParser::LoadItemData()
{
    try
    {
        GConsoleLogger->WriteStdOut(Color::GREEN, L"아이템 데이터 로딩 시작: Item_data.json\n");

        auto itemDatas = JsonDataParser::ParseMapData<ItemData>(
            "Item_data.json",
            [](const rapidjson::Value& json) { return JsonToItemData(json); },
            [](const rapidjson::Value& json) { return ExtractItemId(json); }
        );

        GConsoleLogger->WriteStdOut(Color::GREEN, L"아이템 데이터 로딩 완료: %d개의 아이템 개수\n", static_cast<int>(itemDatas.size()));

        // 로드된 아이템 정보 출력
        for (const auto& [itemId, data] : itemDatas)
        {
            GConsoleLogger->WriteStdOut(Color::WHITE,
                L"  - Item[%d] %s: Description=%s, IsStackable=%s, MaxStack=%d, EItemType=%d\n",
                data.itemId,
                StrToWstr(data.name).c_str(),
                StrToWstr(data.description).c_str(),
                data.isStackable ? L"True" : L"False",
                data.maxStack,
                static_cast<int>(data.itemType) // int
            );
        }

        return itemDatas;
    }
    catch (const std::exception& e)
    {
        GConsoleLogger->WriteStdOut(Color::RED, L"아이템 데이터 로딩 실패: %s\n", StrToWstr(e.what()).c_str());
        throw;
    }
}

ItemData ItemDataParser::JsonToItemData(const rapidjson::Value& json)
{
    ItemData itemData;

    // Json 필드명 -> ItemData 필드 매핑
    itemData.itemId         = JsonDataParser::SafeGetInt(json, "itemId");
    itemData.name           = JsonDataParser::SafeGetString(json, "name");
    itemData.description    = JsonDataParser::SafeGetString(json, "description");
    itemData.isStackable    = JsonDataParser::SafeGetBool(json, "isStackable");
    itemData.maxStack       = JsonDataParser::SafeGetInt(json, "maxStack");
    itemData.itemType       = ItemDataParser::SafeGetItemType(json, "itemType");
       
    return itemData;
}

int ItemDataParser::ExtractItemId(const rapidjson::Value& json)
{
    return JsonDataParser::SafeGetInt(json, "itemId");
}

Protocol::EItemType ItemDataParser::SafeGetItemType(const rapidjson::Value& value, const std::string& fieldName)
{
    if (!value.HasMember(fieldName.c_str()))
    {
        throw std::runtime_error("JSON에서 필수 필드를 찾을 수 없습니다. " + fieldName);
    }

    const auto& field = value[fieldName.c_str()];

    if (!field.IsString())
    {
        throw std::runtime_error("잘못된 데이터 타입 (필드: " + fieldName + ", 문자열이 아님)");
    }

    auto fieldStr = static_cast<string>(field.GetString());
    
    if      (fieldStr == "UNKNOWN")     return Protocol::EItemType::ITEM_TYPE_UNKNOWN;
    else if (fieldStr == "CONSUMABLE")  return Protocol::EItemType::ITEM_TYPE_CONSUMABLE;
    else if (fieldStr == "EQUIPMENT")   return Protocol::EItemType::ITEM_TYPE_EQUIPMENT;
    else if (fieldStr == "QUEST")       return Protocol::EItemType::ITEM_TYPE_QUEST;
    else if (fieldStr == "MISC")        return Protocol::EItemType::ITEM_TYPE_MISC;

}
