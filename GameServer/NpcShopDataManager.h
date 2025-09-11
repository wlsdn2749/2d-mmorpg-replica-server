#pragma once

struct ShopItem
{
	int itemId;
	int quantity; // 판매 수량 -1이면 무제한
	int price; // 한 개당 가격
};

struct ShopData
{
	int shopId;
	std::string shopName;
	std::vector<ShopItem> items;
};

class NpcShopDataManager
{
	// shopId -> ShopData 데이터 매핑
	static std::unordered_map<int, ShopData> _shopDataMap;

	// 데이터를 shopDataMap으로 로드
	static void LoadShopData();
	static const ShopData* GetShopData(int shopId);
};

