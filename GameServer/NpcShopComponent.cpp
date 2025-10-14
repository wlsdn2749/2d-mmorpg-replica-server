#include "pch.h"
#include "NpcShopComponent.h"

NpcShopComponent::NpcShopComponent(int shopId)
	: _shopId(shopId)
{
	Initialize(shopId);
}

void NpcShopComponent::Initialize(int shopId)
{
	// ShopManager 상점 데이터 로드
	const auto* shopData =  ShopManager::Instance().GetShopData(shopId);
	if(!shopData) return;
	_myShopData = *shopData;


	// ShopManager로부터 상점 아이템 데이터 로드

	const auto* shopItemDatas = ShopManager::Instance().GetShopItems(shopId);
	if(!shopItemDatas) return;

	// 복사 (상점 데이터)
	for (const auto& [itemId, shopItemData] : *shopItemDatas)
	{
		_myShopItems[itemId] = *shopItemData;
	}
}

void NpcShopComponent::ShowShop(PlayerRef player)
{
	// _myShopItemsbyPlayer에 플레이어 정보가 없다면?
	// 새로 생성해서 unordered_map에 넣기 
	auto playerId = player->playerId;

	auto it = _myShopItemsbyPlayer.find(playerId);
	if (it == _myShopItemsbyPlayer.end())
	{
		_myShopItemsbyPlayer[playerId] = _myShopItems;
	}

	// S_NpcShopOpen 패킷 생성 후 전송
	// _item -> ShopItemInfo 패킷 변환
	SendShopItemInfo(player);

}

bool NpcShopComponent::ProcessPurchase(PlayerRef player, int itemId, int quantity)
{
	auto playerId = player->playerId;
	auto& shopItems = _myShopItemsbyPlayer[playerId];
	
	// 2. 아이템 가격 계산
	auto buyPrice = shopItems[itemId].buyPrice;
	// 3. 플레이어 금액 체크 (Player 클래스에 money 필드 필요?)
	auto needPlayerPrice = buyPrice * quantity;
	if(player->Money() < needPlayerPrice) // 돈이 부족한경우 에러
		return false; // TODO LOG?

	// 4. 인벤토리 공간 체크
	if(player->GetInventory().IsFull()) // 인벤토리가 이미 가득 찬경우 에러
		return false; // TODO LOG


	// 5. 금액 차감 + 아이템 지급
	player->SetMoney(player->Money() - needPlayerPrice);
	player->AddItem(itemId, quantity);
	// AddItem시 알아서 UpdatePkt 전송

	return true;
}

bool NpcShopComponent::ProcessSell(PlayerRef player, int itemId, int quantity)
{
	// TODO 
	// 1. 플레이어 확인
	auto playerId  = player->playerId;
	// 2. 팔고자하는 아이템 확인 (인벤토리 존재 유무)
	// itemId의 아이템을 quantity만큼 가지고 있는가?
	if(player->GetInventory().FindItemCount(itemId) < quantity)
		return false;

	// 3. 팔고자하는 가격 확인
	auto sellPrice = ItemManager::Instance().GetSellPrice(itemId);
	auto totalSellPRice = sellPrice * quantity;

	// 4. 인벤토리에서 아이템 제거 및 가격 추가
	player->RemoveItemById(itemId, quantity);
	
	return false;
}

Protocol::ShopItemInfo NpcShopComponent::GetShopItemInfo(const ShopItemData& shopItemData)
{
	Protocol::ShopItemInfo info;
	info.set_itemid(shopItemData.itemId);
	info.set_quantity(shopItemData.stock);
	info.set_price(shopItemData.buyPrice);

	return info;
}

void NpcShopComponent::SendShopItemInfo(PlayerRef player)
{
	if(!player) return;

	Protocol::S_NpcShopOpen pkt;
	pkt.set_npcid(1); //TODO npcId를 얻어와야함
	pkt.set_shopid(_shopId);

	auto playerId = player->playerId;
	auto& shopItems = _myShopItemsbyPlayer[playerId];// 플레이어별로 가지는 상점아이템
	for (const auto& [itemId, shopItem] : shopItems)
	{
		*pkt.add_items() = GetShopItemInfo(shopItem);
	}

	if (auto sess = player->ownerSession.lock())
	{
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
		sess->Send(sendBuffer);
	}
}
