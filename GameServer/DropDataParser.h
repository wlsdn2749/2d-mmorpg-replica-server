#pragma once
#include "JsonDataParser.h"
#include "DropManager.h"

/*--------------------------
	Drop_data.json을 파싱하는 특화 클래스
	Google Sheets에서 생성된 아이템 데이터를 DropItemInfo 구조체로 변환
--------------------------*/
class DropDataParser
{
public:
	// Drop_data.json파일에서 모든 데이터를 로드
	// Id를 Key로하는 DropItemInfoMap 반환
	static std::unordered_map<int, DropItemInfo> LoadDropItemData();


	// Json Value에서 DropItemInfo로 변환
	// LoadDropItemData는 이 함수를 호출함
	static DropItemInfo JsonToDropItemInfo(const rapidjson::Value& json);

private:
	// Id 추출
	static int ExtractId(const rapidjson::Value& json);
};

