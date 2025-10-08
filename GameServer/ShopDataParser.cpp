#include "pch.h"
#include "ShopDataParser.h"

std::unordered_map<int, std::unique_ptr<ShopData>> ShopDataParser::LoadShopData()
{
	try
	{
		GConsoleLogger->WriteStdOut(Color::GREEN, L"Shop data loading started: Shop_data.json\n");

		auto shopDataArray = JsonDataParser::ParseArrayData<ShopData>(
			"Shop_data.json",
			[](const rapidjson::Value& json) { return JsonToShopData(json); }
		);

		std::unordered_map<int, std::unique_ptr<ShopData>> result;

		for (auto& shopData : shopDataArray)
		{
			int shopId = shopData.shopId;
			result[shopId] = std::make_unique<ShopData>(std::move(shopData));
		}

		GConsoleLogger->WriteStdOut(Color::GREEN, L"Shop data loading completed: %d shops loaded\n",
			static_cast<int>(result.size()));

		for (const auto& [shopId, dataPtr] : result)
		{
			GConsoleLogger->WriteStdOut(Color::WHITE,
				L"  - Shop[%d]: Name=%s, Type=%d\n",
				dataPtr->shopId,
				StrToWstr(dataPtr->shopName).c_str(),
				static_cast<int>(dataPtr->shopType)
			);
		}

		return result;
	}
	catch (const std::exception& e)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"Shop data loading failed: %s\n", StrToWstr(e.what()).c_str());
		throw;
	}
}

std::unordered_map<int, std::unordered_map<int, std::unique_ptr<ShopItemData>>> ShopDataParser::LoadShopItemData()
{
	try
	{
		GConsoleLogger->WriteStdOut(Color::GREEN, L"ShopItem data loading started: ShopItem_data.json\n");

		auto shopItemArray = JsonDataParser::ParseArrayData<ShopItemData>(
			"ShopItem_data.json",
			[](const rapidjson::Value& json) { return JsonToShopItemData(json); }
		);

		std::unordered_map<int, std::unordered_map<int, std::unique_ptr<ShopItemData>>> result;

		for (auto& item : shopItemArray)
		{
			int shopId = item.shopId;
			int itemId = item.itemId;
			result[shopId][itemId] = std::make_unique<ShopItemData>(std::move(item));
		}

		GConsoleLogger->WriteStdOut(Color::GREEN, L"ShopItem data loading completed: %d shops with items\n",
			static_cast<int>(result.size()));

		for (const auto& [shopId, itemMap] : result)
		{
			GConsoleLogger->WriteStdOut(Color::WHITE, L"  - Shop[%d] has %d items\n",
				shopId, static_cast<int>(itemMap.size()));

			for (const auto& [itemId, dataPtr] : itemMap)
			{
				GConsoleLogger->WriteStdOut(Color::WHITE,
					L"    - Item[%d]: BuyPrice=%d, Stock=%d, RefreshTime=%d\n",
					dataPtr->itemId,
					dataPtr->buyPrice,
					dataPtr->stock,
					dataPtr->refreshTime
				);
			}
		}

		return result;
	}
	catch (const std::exception& e)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"ShopItem data loading failed: %s\n", StrToWstr(e.what()).c_str());
		throw;
	}
}

ShopData ShopDataParser::JsonToShopData(const rapidjson::Value& json)
{
	ShopData shopData;

	shopData.shopId = JsonDataParser::SafeGetInt(json, "shopId");
	shopData.shopName = JsonDataParser::SafeGetString(json, "shopName");
	shopData.shopType = ShopDataParser::SafeGetShopType(json, "shopType");

	return shopData;
}

ShopItemData ShopDataParser::JsonToShopItemData(const rapidjson::Value& json)
{
	ShopItemData shopItemData;

	shopItemData.shopId = JsonDataParser::SafeGetInt(json, "shopId");
	shopItemData.itemId = JsonDataParser::SafeGetInt(json, "itemId");
	shopItemData.buyPrice = JsonDataParser::SafeGetInt(json, "buyPrice");
	shopItemData.stock = JsonDataParser::SafeGetInt(json, "stock");
	shopItemData.refreshTime = JsonDataParser::SafeGetInt(json, "refreshTime");

	return shopItemData;
}

int ShopDataParser::ExtractShopId(const rapidjson::Value& json)
{
	return JsonDataParser::SafeGetInt(json, "shopId");
}

Protocol::EShopType ShopDataParser::SafeGetShopType(const rapidjson::Value& value, const std::string& fieldName)
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

	if (fieldStr == "GENERAL")      return Protocol::EShopType::SHOP_GENERAL;
	else if (fieldStr == "WEAPON")  return Protocol::EShopType::SHOP_WEAPON;
	else
	{
		throw std::runtime_error("Unknown ShopType: " + fieldStr);
	}
}
