#pragma once
#include <string>
#include <vector>

#include "Protocol.pb.h"

class ShopItemData;

struct ShopData
{
	int shopId {};
	std::string shopName {};
	Protocol::EShopType shopType {};

	ShopData() = default;
	ShopData(int shopId, const std::string& shopName, Protocol::EShopType shopType)
		: shopId(shopId), shopName(shopName), shopType(shopType)
	{ 
	
	}
};

struct ShopItemData
{
	int shopId {}; // 판매하는 상점 Id
	int itemId {}; // 판매하는 아이템 id
	int buyPrice {}; // 구매가격  (1 개당)
	int stock {}; // -1이면 무제한
	int refreshTime {}; // 0이면 상시, 3600 = 1시간

	ShopItemData() = default;
	ShopItemData(int shopId, int itemId, int buyPrice, int stock, int refreshTime)
		: shopId(shopId), itemId(itemId), buyPrice(buyPrice), stock(stock), refreshTime(refreshTime)
	{

	}
};

