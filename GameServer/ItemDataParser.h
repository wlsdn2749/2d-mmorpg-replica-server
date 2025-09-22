#pragma once
#include "JsonDataParser.h"
#include "InventoryCore.h"

/*--------------------------
	Item_data.json을 파싱하는 특화 클래스
	Google Sheets에서 생성된 아이템 데이터를 ItemData 구조체로 변환
--------------------------*/
class ItemDataParser
{
public:
	// Item_data.json파일에서 모든 데이터를 로드
	// ItemId를 Key로하는 ItemDataMap 반환
	static std::unordered_map<int, ItemData> LoadItemData();


	// Json Value에서 ItemData로 변환
	// LoadItemData는 이 함수를 호출함
	static ItemData JsonToItemData(const rapidjson::Value& json);

private:
	static int ExtractItemId(const rapidjson::Value& json);

	// ItemDataParser Specific
	static Protocol::EItemType SafeGetItemType(const rapidjson::Value& value, const std::string& fieldName);
};

