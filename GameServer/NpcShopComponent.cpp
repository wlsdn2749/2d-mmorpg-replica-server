#include "pch.h"
#include "NpcShopComponent.h"

void NpcShopComponent::Initialize(int shopId)
{
	// NpcShopDataManager로부터 상점 데이터 로드
	// _item 벡터에 복사
}

void NpcShopComponent::ShowShop(int playerId)
{
	// S_NpcShopOpen 패킷 생성 후 전송
	// _item -> ShopItemInfo 패킷 변환
}

bool NpcShopComponent::ProcessPurchase(int playerId, int itemId, int quantity)
{
	// 1. 플레이어 찾기
	// 2. 아이템 가격 계산
	// 3. 플레이어 금액 체크 (Player 클래스에 money 필드 필요?)
	// 4. 인벤토리 공간 체크
	// 5. 금액 차감 + 아이템 지급
	// 6. S_InventoryUpdate 패킷 전송
	return false;
}

bool NpcShopComponent::ProcessSell(int playerId, int itemId, int quantity)
{
	// TODO
	return false;
}
