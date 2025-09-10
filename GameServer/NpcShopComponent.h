#pragma once
#include "NpcShopDataManager.h"
/// <summary>
/// 상점에 대한 기능을 하는 부분.
/// 데이터는 initialize함수의 NpcShopData를 통해 load한다.
/// </summary>
class NpcShopComponent
{

public:
	void Initialize(int shopId);
	void ShowShop(int playerId);
	bool ProcessPurchase(int playerId, int itemId, int quantity);
	bool ProcessSell(int playerId, int itemId, int quantity);

private:
	int _shopId;
	vector<ShopItem> _items;

private:
};

