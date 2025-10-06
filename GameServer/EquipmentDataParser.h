#pragma once
#include "JsonDataParser.h"
#include "EquipmentCore.h"

/*--------------------------
	equipment_data.json을 파싱하는 특화 클래스
	Google Sheets에서 생성된 장비 데이터를 EquipmentData 구조체로 변환
--------------------------*/
class EquipmentDataParser
{
public:
	// equipment_data.json 파일에서 모든 데이터를 로드
	// ItemId를 Key로하는 EquipmentDataMap 반환
	static std::unordered_map<int, EquipmentData> LoadEquipmentData();

	// Json Value에서 EquipmentData로 변환
	// LoadEquipmentData는 이 함수를 호출함
	static EquipmentData JsonToEquipmentData(const rapidjson::Value& json);

private:
	static int ExtractItemId(const rapidjson::Value& json);

	// EquipmentDataParser Specific
	static Protocol::EEquipmentSlotType SafeGetSlotType(const rapidjson::Value& value, const std::string& fieldName);
};
