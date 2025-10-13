#include "pch.h"
#include "NpcDataParser.h"

std::unordered_map<int, std::unique_ptr<NpcConfig>> NpcDataParser::LoadNpcConfig()
{
	try
	{
		GConsoleLogger->WriteStdOut(Color::GREEN, L"Npc Config loading started: npc_data.json\n");

		auto npcConfigMap = JsonDataParser::ParseMapData<NpcConfig>(
			"Npc_data.json",
			[](const rapidjson::Value& json) { return JsonToNpcConfig(json); },
			[](const rapidjson::Value& json) { return ExtractNpcId(json); }
		);

		GConsoleLogger->WriteStdOut(Color::GREEN, L"Npc Config data loading completed: %d npcs loaded\n",
			static_cast<int>(npcConfigMap.size()));

		std::unordered_map<int, std::unique_ptr<NpcConfig>> result;

		for (auto& [npcId, config] : npcConfigMap)
		{
			result[npcId] = std::make_unique<NpcConfig>(config);
		}

		// 로드된 장비 정보 출력
		for (const auto& [npcId, config] : npcConfigMap)
		{
			GConsoleLogger->WriteStdOut(Color::WHITE,
				L"  - NpcId[%d] %s: Role=%d, MapId=%d, X=%d, Y=%d, dialogId=%d shopId=%d, questIds=%d\n",
				config.npcId,
				StrToWstr(config.name).c_str(),
				config.role,
				config.mapId,
				config.x,
				config.y,
				config.dialogId,
				config.shopId,
				static_cast<int>(config.questIds.size())
			);
		}

		return result;
	}
	catch (const std::exception& e)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"Npc Config 로딩 실패: %s\n", StrToWstr(e.what()).c_str());
		throw;
	}

}

NpcConfig NpcDataParser::JsonToNpcConfig(const rapidjson::Value& json)
{
	NpcConfig npcConfig{};

	// Json 필드명 -> NpcConfig 필드 매핑

	npcConfig.npcId = JsonDataParser::SafeGetInt(json, "npcId");
	npcConfig.name = JsonDataParser::SafeGetString(json, "name");
	npcConfig.role = NpcDataParser::SafeGetNpcRole(json, "role");
	npcConfig.mapId = JsonDataParser::SafeGetInt(json, "mapId");
	npcConfig.x = JsonDataParser::SafeGetInt(json, "x");
	npcConfig.y = JsonDataParser::SafeGetInt(json, "y");
	npcConfig.dialogId = JsonDataParser::SafeGetInt(json, "dialogId");
	npcConfig.shopId = JsonDataParser::SafeGetInt(json, "shopId");
	npcConfig.questIds = NpcDataParser::SafeGetNpcQuestIds(json, "questIds");

	return npcConfig;
}

int NpcDataParser::ExtractNpcId(const rapidjson::Value& json)
{
	return JsonDataParser::SafeGetInt(json, "npcId");
}

ENpcRole NpcDataParser::SafeGetNpcRole(const rapidjson::Value& value, const std::string& fieldName)
{
	if (!value.HasMember(fieldName.c_str()))
	{
		throw std::runtime_error("JSON에서 필수 필드를 찾을 수 없습니다."); 
	}

	const auto& field = value[fieldName.c_str()];

	if (!field.IsString())
	{
		throw std::runtime_error("잘못된 데이터 타입 (필드: " + fieldName + ", 문자열이 아님)");
	}

	auto fieldStr = static_cast<string>(field.GetString());

	if      (fieldStr == "DIALOG")		return ENpcRole::Dialog;
    else if (fieldStr == "SHOP")		return ENpcRole::Shop;
    else if (fieldStr == "QUEST")		return ENpcRole::Quest;
    else if (fieldStr == "MIXED")       return ENpcRole::Mixed;
}

vector<int> NpcDataParser::SafeGetNpcQuestIds(const rapidjson::Value& value, const std::string& fieldName)
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

	vector<int> result {};

	if(field.GetStringLength() == 0)
		return result;

	auto fieldStr = static_cast<string>(field.GetString());

	auto v = StringUtils::split(fieldStr, ",");
	for (const auto& str : v)
	{
		try
		{
			result.push_back(stoi(str));
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error("잘못된 데이터 타입 (데이터: " + str + ", 문자열 데이터 타입이 아님" + e.what());
		}
	}

	return result;
}
