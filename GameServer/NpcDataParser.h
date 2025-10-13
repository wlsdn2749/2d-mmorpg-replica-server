#pragma once

#include "JsonDataParser.h"
#include "Npc.h"

/*-----------------------------
	npc_data.json을 파싱하는 특화 클래스
	Google Sheets에서 생성된 장비 데이터를 NpcConfig 구조체로 변환해야함
-----------------------------*/
class NpcDataParser
{
public:
	// npc_data.json 파일에서 모든 데이터 로드 
	// NpcId를 key로 하는 NpcConfigMap 반환
	static std::unordered_map<int, std::unique_ptr<NpcConfig>> LoadNpcConfig();

	// Json Value에서 NpcConfig로 변환
	// LoadNpcConfig는 이 함수를 호출함
	static NpcConfig JsonToNpcConfig(const rapidjson::Value& json);


private:
	static int ExtractNpcId(const rapidjson::Value& json);

	// NpcDataParser Specific	
	static ENpcRole SafeGetNpcRole(const rapidjson::Value& value, const std::string& fieldName);
	static vector<int> SafeGetNpcQuestIds(const rapidjson::Value& value, const std::string& fieldName);
};

