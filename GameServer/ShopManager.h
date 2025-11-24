#pragma once
#include "ShopCore.h"
#include <unordered_map>
#include <memory>
#include <string>
#include "Singleton.h"

class ShopManager : public Singleton<ShopManager>
{

public:
	// 초기화 및 정리
	bool Initialize();
	void Shutdown();

	// 상점 데이터 조회
	const ShopData* GetShopData(int shopId) const;
	bool IsValidShop(int shopId) const;
	Protocol::EShopType GetShopType(int shopId) const;
	std::string GetShopName(int shopId) const;

	// 상점 아이템 조회
	const ShopItemData* GetShopItemData(int shopId, int itemId) const;
	const std::unordered_map<int, std::unique_ptr<ShopItemData>>* GetShopItems(int shopId) const;
	bool HasShopItem(int shopId, int itemId) const;
	int GetBuyPrice(int shopId, int itemId) const;
	int GetStock(int shopId, int itemId) const;
	int GetRefreshTime(int shopId, int itemId) const;

	// 디버그 및 관리
	void PrintAllShops() const;
	void PrintShopItems(int shopId) const;
	size_t GetShopCount() const;
	size_t GetShopItemCount(int shopId) const;

private:
	// 데이터 추가/제거 헬퍼
	void AddShopData(ShopData&& shopData);
	void AddShopItemData(ShopItemData&& shopItemData);
	void RemoveShopData(int shopId);
	void RemoveShopItemData(int shopId, int itemId);

private:
	std::unordered_map<int, std::unique_ptr<ShopData>> _shopDataMap;
	std::unordered_map<int, std::unordered_map<int, std::unique_ptr<ShopItemData>>> _shopItemDataMap;
	bool _initialized = false;
};

// 편의를 위한 전역 접근 함수들
namespace ShopManagerGlobal
{
	const ShopData* GetShopData(int shopId);
	const ShopItemData* GetShopItemData(int shopId, int itemId);
	bool IsValidShop(int shopId);
	bool HasShopItem(int shopId, int itemId);
}
