#include "pch.h"
#include "ShopManager.h"
#include "ShopDataParser.h"

ShopManager& ShopManager::Instance()
{
	static ShopManager instance;
	return instance;
}

bool ShopManager::Initialize()
{
	if (_initialized)
		return true;

	GConsoleLogger->WriteStdOut(Color::YELLOW, L"ShopManager: Initializing...\n");

	// Shop_data.json에서 상점 데이터 로드 (move)
	_shopDataMap = ShopDataParser::LoadShopData();

	// ShopItem_data.json에서 상점 아이템 데이터 로드 (move)
	_shopItemDataMap = ShopDataParser::LoadShopItemData();

	_initialized = true;

	GConsoleLogger->WriteStdOut(Color::GREEN, L"ShopManager: Initialization complete. Total %d shops loaded.\n",
		static_cast<int>(_shopDataMap.size()));

	return true;
}

void ShopManager::Shutdown()
{
	if (!_initialized)
		return;

	GConsoleLogger->WriteStdOut(Color::YELLOW, L"ShopManager: Shutting down...\n");

	_shopDataMap.clear();
	_shopItemDataMap.clear();
	_initialized = false;

	GConsoleLogger->WriteStdOut(Color::GREEN, L"ShopManager: Shutdown complete.\n");
}

const ShopData* ShopManager::GetShopData(int shopId) const
{
	auto it = _shopDataMap.find(shopId);
	return (it != _shopDataMap.end()) ? it->second.get() : nullptr;
}

bool ShopManager::IsValidShop(int shopId) const
{
	return _shopDataMap.find(shopId) != _shopDataMap.end();
}

Protocol::EShopType ShopManager::GetShopType(int shopId) const
{
	const ShopData* data = GetShopData(shopId);
	return data ? data->shopType : Protocol::EShopType::SHOP_GENERAL;
}

std::string ShopManager::GetShopName(int shopId) const
{
	const ShopData* data = GetShopData(shopId);
	return data ? data->shopName : "";
}

const ShopItemData* ShopManager::GetShopItemData(int shopId, int itemId) const
{
	auto shopIt = _shopItemDataMap.find(shopId);
	if (shopIt == _shopItemDataMap.end())
		return nullptr;

	auto itemIt = shopIt->second.find(itemId);
	return (itemIt != shopIt->second.end()) ? itemIt->second.get() : nullptr;
}

const std::unordered_map<int, std::unique_ptr<ShopItemData>>* ShopManager::GetShopItems(int shopId) const
{
	auto it = _shopItemDataMap.find(shopId);
	return (it != _shopItemDataMap.end()) ? &it->second : nullptr;
}

bool ShopManager::HasShopItem(int shopId, int itemId) const
{
	return GetShopItemData(shopId, itemId) != nullptr;
}

int ShopManager::GetBuyPrice(int shopId, int itemId) const
{
	const ShopItemData* data = GetShopItemData(shopId, itemId);
	return data ? data->buyPrice : 0;
}

int ShopManager::GetStock(int shopId, int itemId) const
{
	const ShopItemData* data = GetShopItemData(shopId, itemId);
	return data ? data->stock : 0;
}

int ShopManager::GetRefreshTime(int shopId, int itemId) const
{
	const ShopItemData* data = GetShopItemData(shopId, itemId);
	return data ? data->refreshTime : 0;
}

void ShopManager::PrintAllShops() const
{
	GConsoleLogger->WriteStdOut(Color::GREEN, L"=== ShopManager: All Shops ===\n");

	for (const auto& [shopId, shopData] : _shopDataMap)
	{
		GConsoleLogger->WriteStdOut(Color::WHITE,
			L"ShopID: %d, Name: %s, Type: %d\n",
			shopData->shopId,
			StrToWstr(shopData->shopName).c_str(),
			static_cast<int>(shopData->shopType)
		);
	}

	GConsoleLogger->WriteStdOut(Color::GREEN, L"Total shops: %d\n", static_cast<int>(_shopDataMap.size()));
}

void ShopManager::PrintShopItems(int shopId) const
{
	const auto* shopItems = GetShopItems(shopId);
	if (!shopItems)
	{
		GConsoleLogger->WriteStdOut(Color::RED, L"ShopManager: Shop %d not found\n", shopId);
		return;
	}

	const ShopData* shopData = GetShopData(shopId);
	GConsoleLogger->WriteStdOut(Color::GREEN, L"=== ShopManager: Items in Shop %d (%s) ===\n",
		shopId, shopData ? StrToWstr(shopData->shopName).c_str() : L"Unknown");

	for (const auto& [itemId, itemDataPtr] : *shopItems)
	{
		GConsoleLogger->WriteStdOut(Color::WHITE,
			L"  ItemID: %d, BuyPrice: %d, Stock: %d, RefreshTime: %d\n",
			itemDataPtr->itemId,
			itemDataPtr->buyPrice,
			itemDataPtr->stock,
			itemDataPtr->refreshTime
		);
	}

	GConsoleLogger->WriteStdOut(Color::GREEN, L"Total items: %d\n", static_cast<int>(shopItems->size()));
}

size_t ShopManager::GetShopCount() const
{
	return _shopDataMap.size();
}

size_t ShopManager::GetShopItemCount(int shopId) const
{
	const auto* shopItems = GetShopItems(shopId);
	return shopItems ? shopItems->size() : 0;
}

void ShopManager::AddShopData(ShopData&& shopData)
{
	int shopId = shopData.shopId;
	_shopDataMap[shopId] = std::make_unique<ShopData>(std::move(shopData));
}

void ShopManager::AddShopItemData(ShopItemData&& shopItemData)
{
	int shopId = shopItemData.shopId;
	int itemId = shopItemData.itemId;
	_shopItemDataMap[shopId][itemId] = std::make_unique<ShopItemData>(std::move(shopItemData));
}

void ShopManager::RemoveShopData(int shopId)
{
	_shopDataMap.erase(shopId);
}

void ShopManager::RemoveShopItemData(int shopId, int itemId)
{
	auto shopIt = _shopItemDataMap.find(shopId);
	if (shopIt != _shopItemDataMap.end())
	{
		shopIt->second.erase(itemId);
	}
}

// 편의를 위한 전역 접근 함수들
namespace ShopManagerGlobal
{
	const ShopData* GetShopData(int shopId)
	{
		return ShopManager::Instance().GetShopData(shopId);
	}

	const ShopItemData* GetShopItemData(int shopId, int itemId)
	{
		return ShopManager::Instance().GetShopItemData(shopId, itemId);
	}

	bool IsValidShop(int shopId)
	{
		return ShopManager::Instance().IsValidShop(shopId);
	}

	bool HasShopItem(int shopId, int itemId)
	{
		return ShopManager::Instance().HasShopItem(shopId, itemId);
	}
}
