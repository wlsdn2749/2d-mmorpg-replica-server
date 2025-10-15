#pragma once
#include "JsonDataParser.h"
#include "ShopCore.h"
#include <memory>

/*------------------------------------
	Shop_data.json과 ShopItem_data.json를 파싱하는 특화 클래스
	Google Sheets에서 생성된 아이템 데이터를 ShopData와 ShopItemData 구조체로 파싱
------------------------------------*/
class ShopDataParser
{
public:
	// Shop_data.json 파일에서 모든 데이터를 로드
	// ShopId를 Key로 하는 ShopDataMap을 반환 (unique_ptr)
	static std::unordered_map<int, std::unique_ptr<ShopData>> LoadShopData();

	// ShopItem_data.json 파일에서 모든 데이터를 로드
	// ShopId -> (ItemId -> ShopItemData) 중첩 맵 구조로 반환 (unique_ptr)
	static std::unordered_map<int, std::unordered_map<int, std::unique_ptr<ShopItemData>>> LoadShopItemData();

	// Json Value에서 ShopData로 변환
	static ShopData JsonToShopData(const rapidjson::Value& json);

	// Json Value에서 ShopItemData로 변환
	static ShopItemData JsonToShopItemData(const rapidjson::Value& json);

private:
	static int ExtractShopId(const rapidjson::Value& json);

	// ShopDataParser Specific
	static Protocol::EShopType SafeGetShopType(const rapidjson::Value& value, const std::string& fieldName);
};

