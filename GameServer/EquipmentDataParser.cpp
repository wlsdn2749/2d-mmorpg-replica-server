#include "pch.h"
#include "EquipmentDataParser.h"

std::unordered_map<int, std::unique_ptr<EquipmentData>> EquipmentDataParser::LoadEquipmentData()
{
	try
	{
		GConsoleLogger->WriteStdOut(Color::GREEN, L"Equipment data loading started: equipment_data.json\n");

		auto equipmentDataArray = JsonDataParser::ParseArrayData<EquipmentData>(
			"Equipment_data.json",
			[](const rapidjson::Value& json) { return JsonToEquipmentData(json); }
		);

		std::unordered_map<int, std::unique_ptr<EquipmentData>> result;

		for (auto& equipmentData : equipmentDataArray)
		{
			int itemId = equipmentData.itemId;
			result[itemId] = std::make_unique<EquipmentData>(std::move(equipmentData));
		}

		GConsoleLogger->WriteStdOut(Color::GREEN, L"Equipment data loading completed: %d equipments loaded\n",
			static_cast<int>(result.size()));

		// 로드된 장비 정보 출력
		for (const auto& [itemId, dataPtr] : result)
		{
			GConsoleLogger->WriteStdOut(Color::WHITE,
				L"  - Equipment[%d]: SlotType=%d, MinLevel=%d, Atk=%d, Def=%d, MaxHp=%d\n",
				dataPtr->itemId,
				static_cast<int>(dataPtr->slotType),
				dataPtr->minLevel,
				dataPtr->atk,
				dataPtr->def,
				dataPtr->maxHp
			);
		}

		return result;
	}
	catch (const std::exception& e)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"Equipment data loading failed: %s\n", StrToWstr(e.what()).c_str());
		throw;
	}
}

EquipmentData EquipmentDataParser::JsonToEquipmentData(const rapidjson::Value& json)
{
	EquipmentData equipmentData;

	// Json 필드명 -> EquipmentData 필드 매핑
	equipmentData.itemId = JsonDataParser::SafeGetInt(json, "itemId");
	equipmentData.slotType = EquipmentDataParser::SafeGetSlotType(json, "slotType");
	equipmentData.minLevel = JsonDataParser::SafeGetInt(json, "minLevel");
	equipmentData.atk = JsonDataParser::SafeGetInt(json, "atk");
	equipmentData.def = JsonDataParser::SafeGetInt(json, "def");
	equipmentData.maxHp = JsonDataParser::SafeGetInt(json, "maxHp");

	return equipmentData;
}

int EquipmentDataParser::ExtractItemId(const rapidjson::Value& json)
{
	return JsonDataParser::SafeGetInt(json, "itemId");
}

Protocol::EEquipmentSlotType EquipmentDataParser::SafeGetSlotType(const rapidjson::Value& value, const std::string& fieldName)
{
	if (!value.HasMember(fieldName.c_str()))
	{
		throw std::runtime_error("JSON required field not found: " + fieldName);
	}

	const auto& field = value[fieldName.c_str()];

	if (!field.IsString())
	{
		throw std::runtime_error("Invalid data type (field: " + fieldName + ", not a string)");
	}

	auto fieldStr = static_cast<string>(field.GetString());

	if (fieldStr == "WEAPON")       return Protocol::EEquipmentSlotType::EQUIPMENT_WEAPON;
	else if (fieldStr == "HELMET")  return Protocol::EEquipmentSlotType::EQUIPMENT_HELMET;
	else if (fieldStr == "ARMOR")   return Protocol::EEquipmentSlotType::EQUIPMENT_ARMOR;
	else if (fieldStr == "PANTS")   return Protocol::EEquipmentSlotType::EQUIPMENT_PANTS;
	else
	{
		throw std::runtime_error("Unknown SlotType: " + fieldStr);
	}
}
