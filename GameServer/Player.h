#pragma once
#include "TypeCore.h"       // EntityId, EntityKind
#include "GeometryCore.h"   // Pos2, Dir
#include "EntityCore.h"     // EntityCore
#include "CharacterRepository.h"
#include "InventorySystem.h"
#include "InventoryRepository.h"
#include "ItemManager.h"

#include "EquipmentSystem.h"
#include "EquipmentRepository.h"

#include "GameSession.h"
#include "ClientPacketHandler.h"

class Room; // 전방 선언

struct PendingRoomChange {
	bool active = false;
	int transitionId = 0;
	int dstMapId = 0;
	int dstPortalId = 0;
};

class Player
{
public:
	bool operator==(const Player& rhs) const { return this->playerId == rhs.playerId; }
	bool operator!=(const Player& rhs) const { return this->playerId != rhs.playerId; }
	/*----------------------------
		Player State
	----------------------------*/
public:
	enum class PlayerState : uint8
	{
		Alive, // 생존
		Dead, // 죽음
	};

	PlayerState	GetPlayerState() const { return _playerState.load(std::memory_order_acquire); }
	void		SetPlayerState(PlayerState s) { _playerState.store(s, std::memory_order_release); }

	bool		IsDead() const { return _playerState == Player::PlayerState::Dead; }

private:
	Atomic<PlayerState> _playerState{ PlayerState::Alive };


	/*----------------------------
		Player Fixed Data
	----------------------------*/

public:
	EntityId			playerId{ 0 }; // DB에서 CharacterId임
	string				username;
	Protocol::EGender	gender{ 0 }; // gender
	Protocol::ERegion	region{ 0 }; // region

	/*------------------------------------------
		Player Move Data (Core) / (위치, 방향)
	------------------------------------------*/
public:
	EntityCore core{
		/*id   */ 0,
		/*kind */ EntityKind::Player,
		/*pos  */ {0,0},
		/*dir  */ Protocol::EDirection::DIR_DOWN
	};

	inline int PosX() const { return core.pos.x; }
	inline int PosY() const { return core.pos.y; }
	inline Protocol::EDirection Dir() const { return core.dir; }
	inline Pos2 GetPos() const { return { core.pos.x, core.pos.y }; }
	inline void SetPos(int x, int y) { core.pos = { x,y }; }
	inline void SetDir(Protocol::EDirection d) { core.dir = d; }
	inline void ResetPos() { core.pos.x = 0, core.pos.y = 0; }
	/*----------------------------------------
		Player Room 데이터
	----------------------------------------*/

public:
	inline void SetLastRoomId(int lastRoomId) { _lastRoomId = lastRoomId; }
	inline int LastRoomId() const { return _lastRoomId; }

private:
	int _lastRoomId{ 0 };

	/*-------------------------------
		HP 등 전투 수치 + 레벨
	-------------------------------*/
public:
	inline int MaxHp() const { return _maxHp; }
	inline int Hp() const { return _hp; }
	inline int Atk() const { return _atk; }
	inline int Def() const { return _def; }
	inline int Level() const { return _level; }
	inline int Exp() const { return _curExp; }
	inline int MaxExp() const { return _maxExp; }
	inline int Money() const { return _money; };

	inline void SetMaxHp(int maxHp) { _maxHp = maxHp; }
	inline void SetHp(int hp) { _hp = hp; }
	inline void SetAtk(int atk) { _atk = atk; }
	inline void SetDef(int def) { _def = def; }
	inline void SetLevel(int level) { _level = level; }
	inline void SetExp(int exp) { _curExp = exp; }
	inline void SetMoney(int money) { _money = money; }
	inline void SetMaxExp(int maxExp) { _maxExp = maxExp; }

	inline void SetBaseMaxHp(int baseMaxHp) { _baseMaxHp = baseMaxHp; }
	inline void SetBaseAtk(int baseAtk) { _baseAtk = baseAtk; }
	inline void SetBaseDef(int baseDef) { _baseDef = baseDef; }

	inline void AddMoney(int money) { _money += money; } // TODO 나중에 돈이 변경될때 DB에 업데이트하는 로직이 있어야함
	inline void AddLevel(int level = 1) { _level += level; }
	inline void AddExp(int exp)
	{
		// 나중에 경험치 시스템으로 분할
		_curExp += exp;

		const int pendingAddLevel = static_cast<int>(_curExp / _maxExp);
		const int remainingExp = static_cast<int>(_curExp % _maxExp);
		AddLevel(pendingAddLevel);
		SetExp(remainingExp);
	}

	bool ApplyDamage(int dmg, int srcMonsterId) {
		_hp = std::max(0, _hp - std::max(0, dmg));
		return (_hp == 0); // dead?
	}

	void ResetToRespawnState()
	{
		ResetPos();
		SetHp(MaxHp()); // 피 Max로
		SetPlayerState(PlayerState::Alive);
	}

	std::unique_ptr<Protocol::PlayerStatInfo> GetPlayerStatInfo() const;

	// 장비 스탯 재계산
	void RecalculateStats();

public: // TODO 나중에 private로 수정 필
	// 기본 스탯 (레벨업 등으로 얻는 순수 스탯)
	int _baseMaxHp{ 30 };
	int _baseAtk{ 10 };
	int _baseDef{ 5 };

	// 최종 스탯 (기본 + 장비 보너스)
	int _hp{ 30 };
	int _maxHp{ 30 };
	int _atk{ 10 };
	int _def{ 5 };

	int _level{ 1 };
	int _curExp{ 0 };
	int _maxExp{ 500 };
	int _money{ 0 };

	/*--------------------------
		Level System Component
	--------------------------*/
public:


	/*---------------------------------
		Inventory System
	----------------------------------*/
public:

	// 인벤토리 접근자
	InventorySystem& GetInventory() { return _inventory; }
	const InventorySystem& GetInventory() const { return _inventory; }

	// 인벤토리 DB 연동
	std::future<void> LoadInventoryFromDB();
	std::future<void> SaveInventoryToDB();

	// 인벤토리 편의 메서드
	EAddItemResult AddItem(int itemId, int count, int equipmentInstanceId = 0) {
		// 1. ItemManager에서 아이템 타입 확인
		const ItemData* itemData = ItemManager::Instance().GetItemData(itemId);
		if (!itemData) return EAddItemResult::InvalidItem;

		int finalEquipmentInstanceId = equipmentInstanceId;

		// 2. 장비 타입이고 equipmentInstanceId == 0이면 새로 생성
		if (itemData->itemType == Protocol::EItemType::ITEM_TYPE_EQUIPMENT && equipmentInstanceId == 0)
		{
			// DB에 EquipmentInstance 생성
			auto future = EquipmentRepository::CreateEquipmentInstanceAsync(itemId, 0); // enhancementLevel = 0
			finalEquipmentInstanceId = future.get(); // 생성된 ID 받기
		}

		// 3. 인벤토리에 추가 (equipmentInstanceId 포함)
		auto result = _inventory.AddItem(itemId, count, finalEquipmentInstanceId);
		if (result == EAddItemResult::Success) {
			SaveChangedSlotsToDB();
			SendUpdateInventoryPkt(/*ItemId*/ itemId, /*slotIndex*/ -1);
		}
		return result;
	}

	ERemoveItemResult RemoveItemById(int itemId, int count)
	{
		auto result = _inventory.RemoveItemById(itemId, count);
		if (result == ERemoveItemResult::Success)
		{
			SaveChangedSlotsToDB();
		}
		return result;
	}

	ERemoveItemResult RemoveItem(int slotIndex, int count, bool deleteEquipmentInstance = true) {
		// 제거 전에 장비 인스턴스 ID 확인
		const ItemSlot& slot = _inventory.GetSlot(slotIndex);
		int equipmentInstanceId = slot.equipmentInstanceId;
		int remainingCount = slot.count - count;

		auto result = _inventory.RemoveItem(slotIndex, count);
		if (result == ERemoveItemResult::Success) {
			// 슬롯이 완전히 비워졌고 장비 인스턴스가 있었다면 DB에서 삭제
			if (deleteEquipmentInstance && remainingCount <= 0 && equipmentInstanceId > 0) {
				EquipmentRepository::DeleteEquipmentInstanceAsync(equipmentInstanceId);
			}
			SaveSlotToDB(slotIndex);
			SendUpdateInventoryPkt(/*ItemId*/ -1, /*slotIndex*/ slotIndex);
		}
		return result;
	}

	EUseItemResult UseItem(int slotIndex) {
		auto result = _inventory.UseItem(slotIndex);
		if (result == EUseItemResult::Success) {
			SaveSlotToDB(slotIndex);
			SendUpdateInventoryPkt(/*ItemId*/ -1, /*slotIndex*/ slotIndex);
		}
		return result;
	}

	// 아이템 사용을 위한 헬퍼 함수 (아이템 정보 반환)
	int GetItemIdFromSlot(int slotIndex) const {
		const ItemSlot& slot = _inventory.GetSlot(slotIndex);
		return slot.IsEmpty() ? 0 : slot.itemId;
	}

private:
	void SaveSlotToDB(int slotIndex) 
	{
		int characterId = static_cast<int>(playerId);
		
		const ItemSlot& slot = _inventory.GetSlot(slotIndex);

		if (slot.IsEmpty()) {
			// 빈 슬롯이면 DB에서 삭제
			InventoryRepository::DeleteInventorySlotAsync(characterId, slotIndex);
		}
		else {
			// 슬롯에 아이템이 있으면 저장
			InventoryRepository::SaveInventorySlotAsync(characterId, slot);
		}
	}

	// AddItem은 여러 슬롯에 영향을 줄 수 있으므로 변경된 슬롯들만 저장
	void SaveChangedSlotsToDB() {
		// AddItem 구현이 복잡하므로 현재는 전체 저장
		// TODO: 나중에 변경된 슬롯만 추적하도록 개선
		SaveInventoryToDB();
	}
	void SendUpdateInventoryPkt(int itemId = -1, int slotIndex = -1)
	{
		if(itemId == -1 && slotIndex == -1) return;

		if (itemId != -1)
		{
			return SendUpdateInventoryPktitemId(itemId);
		}
		else if (slotIndex != -1)
		{
			return SendUpdateInventoryPktSlotIndex(slotIndex);
		}
	}

	void SendUpdateInventoryPktitemId(int itemId)
	{
		// 인벤토리 업데이트 전송
		Protocol::S_InventoryUpdate updatePkt;
		auto slots = GetInventory().ToProtocolSlots();
		for (const auto& slot : slots)
		{
			if (slot.itemid() == static_cast<int>(itemId))
			{
				*updatePkt.add_changedslots() = slot;
				break;
			}
		}

		auto updateBuffer = ClientPacketHandler::MakeSendBuffer(updatePkt);

		if (auto session = ownerSession.lock())
			session->Send(updateBuffer);

		return;
	}

	void SendUpdateInventoryPktSlotIndex(int slotIndex)
	{
		Protocol::S_InventoryUpdate updatePkt;
		auto slots = GetInventory().ToProtocolSlots();
		*updatePkt.add_changedslots() = slots[slotIndex];
				
		auto updateBuffer = ClientPacketHandler::MakeSendBuffer(updatePkt);

		if (auto session = ownerSession.lock())
			session->Send(updateBuffer);

		return;
	}

	void SendInventory()
	{
		// 플레이어의 인벤토리 정보를 가져와서 응답 패킷 생성
		const InventorySystem& inventory = GetInventory();
		auto slots = inventory.ToProtocolSlots();

		Protocol::S_InventoryReply replyPkt;
		for (const auto& slotInfo : slots)
		{
			*replyPkt.add_slots() = slotInfo;
		}

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(replyPkt);

		if (auto session = ownerSession.lock())
			session->Send(sendBuffer);

		return;
	}
private:
	InventorySystem _inventory;


	/*---------------------------------
		DB Packer
	----------------------------------*/
public:
	void GetCharacterStat(CharacterRepository::CharacterStat& outStat) const;
	void LoadCharacterStat(const CharacterRepository::CharacterStat& stat);
	std::future<void> SaveCharacterToDB();

	/*---------------------------------
		Player Room Transitioning Data
	---------------------------------*/
public:
	int NextTransitionId() { return ++_lastTransitionId; }

	void BeginRoomChangeState(int tid, int dstMapId, int dstPortalId) {
		_transferring = true;
		_pending.active = true;
		_pending.transitionId = tid;
		_pending.dstMapId = dstMapId;
		_pending.dstPortalId = dstPortalId;
	}
	void ClearRoomChangeState() {
		_transferring = false;
		_pending = {};
	}

	bool IsTransferring() const { return _transferring; }
	const PendingRoomChange& PendingChange() const { return _pending; }

private:
	int _lastTransitionId = 0;
	bool _transferring = false;
	PendingRoomChange _pending;

	/*----------------------------
		Player Runtime Links
	----------------------------*/
public:
	// Room은 Player보다 오래 살아 있을 수 있으므로 weak_ptr
	std::weak_ptr<Room> room;

	// Session은 Player보다 오래 살아 있을 수 있으므로 weak_ptr
	std::weak_ptr<GameSession> ownerSession;

public:
	void SetRoom(const shared_ptr<Room>& r) { room = r; }
	shared_ptr<Room> GetRoom() const { return room.lock(); }

	/*-----------------------
		Party Data
	-----------------------*/
public:
	inline int32 GetPartyId() const { return _partyId; }
	inline void SetPartyId(int32 partyId) { _partyId = partyId; }
	inline bool IsInParty() const { return _partyId != 0; }

private:
	int32 _partyId = 0; // 파티 ID (0 = 파티 없음)

	/*-----------------------
		Equipment Data
	-----------------------*/
public:
	// 장비 접근자
	EquipmentSystem& GetEquipment() { return _equipment; }
	const EquipmentSystem& GetEquipment() const { return _equipment; }

	// 장비 DB 연동
	std::future<void> LoadEquipmentFromDB();
	std::future<void> SaveEquipmentToDB();

	// 장비 편의 메서드
	EEquipItemResult EquipItem(Protocol::EEquipmentSlotType slotType, int itemId, int equipmentInstanceId = 0, int enhancementLevel = 0) {
		auto result = _equipment.EquipItem(slotType, itemId, equipmentInstanceId, enhancementLevel);
		if (result == EEquipItemResult::Success) {
			RecalculateStats();
			SaveEquipmentSlotToDB(slotType);
		}
		return result;
	}

	void UnequipItem(Protocol::EEquipmentSlotType slotType) {
		_equipment.UnequipItem(slotType);
		RecalculateStats();
		SaveEquipmentSlotToDB(slotType);
	}

	// 장착된 아이템 정보를 반환
	int GetEquipmentInstanceIdFromSlot(Protocol::EEquipmentSlotType slotType) const {
		const EquipmentSlot& slot = _equipment.GetEquipmentSlot(slotType);
		return slot.IsEmpty() ? 0 : slot.equipmentInstanceId;
	}

	int GetEquipmentItemIdFromSlot(Protocol::EEquipmentSlotType slotType) const {
		const EquipmentSlot& slot = _equipment.GetEquipmentSlot(slotType);
		return slot.IsEmpty() ? 0 : slot.itemId;
	}

private:
	void SaveEquipmentSlotToDB(Protocol::EEquipmentSlotType slotType) {
		EntityId characterId = playerId;
		const EquipmentSlot& slot = _equipment.GetEquipmentSlot(slotType);

		if (slot.IsEmpty()) {
			// 빈 슬롯이면 DB에서 삭제
			EquipmentRepository::DeleteCharacterEquipmentAsync(characterId, slotType);
		}
		else {
			// 슬롯에 장비가 있으면 저장
			EquipmentRepository::UpsertCharacterEquipmentAsync(characterId, slotType, slot.equipmentInstanceId);
		}
	}

private:
	EquipmentSystem _equipment;

};

