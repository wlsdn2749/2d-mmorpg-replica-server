#pragma once

#include "ShopCore.h"

#include "ShopManager.h"

#include "Player.h"

#include "ClientPacketHandler.h"

#include "GameSession.h"

/// <summary>
/// 상점에 대한 기능을 하는 부분.
/// 데이터는 initialize함수의 NpcShopData를 통해 load한다.
/// </summary>
class NpcShopComponent
{
public:
	NpcShopComponent() = default;
	NpcShopComponent(int shopId);
	~NpcShopComponent() = default;

public:
	void Initialize(int shopId);
	void ShowShop(PlayerRef player);
	bool ProcessPurchase(PlayerRef player, int itemId, int quantity);
	bool ProcessSell(PlayerRef player, int itemId, int quantity);

private:
	int _shopId;
	ShopData _myShopData;
	std::unordered_map<int, ShopItemData> _myShopItems; // 이건 고정된 아이템 가판
	std::unordered_map<int, std::unordered_map<int, ShopItemData>> _myShopItemsbyPlayer;


private:
	Protocol::ShopItemInfo GetShopItemInfo(const ShopItemData& shopItemData);
	void SendShopItemInfo(PlayerRef player);
};

