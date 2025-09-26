#pragma once
#include "TypeCore.h"       // EntityId, EntityKind
#include "GeometryCore.h"   // Pos2, Dir
#include "EntityCore.h"     // EntityCore
#include "CharacterRepository.h"
#include "InventorySystem.h"
#include "InventoryRepository.h"
#include "ItemManager.h"
class Room; // 전방 선언

struct PendingRoomChange {
	bool active = false;
	int transitionId = 0;
	int dstMapId = 0;
	int dstPortalId = 0;
};

class Player
{
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

	bool		IsDead() const {return _playerState == Player::PlayerState::Dead;}

private:
	Atomic<PlayerState> _playerState{ PlayerState::Alive };


/*----------------------------
	Player Fixed Data
----------------------------*/

public:
	EntityId			playerId { 0 }; // DB에서 CharacterId임
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

	inline int PosX() const {return core.pos.x;}
	inline int PosY() const {return core.pos.y;}
	inline Protocol::EDirection Dir() const {return core.dir; }
	inline Pos2 GetPos() const { return { core.pos.x, core.pos.y }; }
	inline void SetPos(int x, int y) { core.pos = { x,y }; }
	inline void SetDir(Protocol::EDirection d) { core.dir = d; }
	inline void ResetPos() { core.pos.x = 0, core.pos.y = 0;}
/*----------------------------------------
	Player Room 데이터 
----------------------------------------*/

public:
	inline void SetLastRoomId(int lastRoomId) { _lastRoomId = lastRoomId; }
	inline int LastRoomId() const {return _lastRoomId; } 

private:
	int _lastRoomId { 0 };

/*-------------------------------
	HP 등 전투 수치 + 레벨
-------------------------------*/
public:
	inline int MaxHp() const {return _maxHp;}
	inline int Hp() const { return _hp; }
	inline int Atk() const { return _atk; }
	inline int Def() const {return _def; }
	inline int Level() const {return _level; }
	inline int Exp() const {return _curExp; }
	inline int MaxExp() const {return _maxExp;}
	inline int Money() const {return _money; };

	inline void SetMaxHp(int maxHp) {_maxHp = maxHp;}
	inline void SetHp(int hp) {_hp = hp; }
	inline void SetAtk(int atk) {_atk = atk;}
	inline void SetDef(int def) {_def = def;}
	inline void SetLevel(int level) {_level = level;}
	inline void SetExp(int exp) {_curExp = exp;}
	inline void SetMoney(int money) {_money = money;}
	inline void SetMaxExp(int maxExp) {_maxExp = maxExp;}

	inline void AddMoney(int money) {_money += money; }
	inline void AddLevel(int level = 1) {_level += level; }
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
	

public: // TODO 나중에 private로 수정 필
	int _hp { 30 };
	int _maxHp { 30 };
	int _atk { 10 };
	int _def { 5 };
	int _level { 1 };
	int _curExp { 0 };
	int _maxExp { 500 };
	int _money { 0 };

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
	EAddItemResult AddItem(int itemId, int count) {
		auto result = _inventory.AddItem(itemId, count);
		if (result == EAddItemResult::Success) {
			SaveChangedSlotsToDB();
		}
		return result;
	}

	ERemoveItemResult RemoveItem(int slotIndex, int count) {
		auto result = _inventory.RemoveItem(slotIndex, count);
		if (result == ERemoveItemResult::Success) {
			SaveSlotToDB(slotIndex);
		}
		return result;
	}
	
	EUseItemResult UseItem(int slotIndex) {
		auto result = _inventory.UseItem(slotIndex);
		if (result == EUseItemResult::Success) {
			SaveSlotToDB(slotIndex);
		}
		return result;
	}

	// 아이템 사용을 위한 헬퍼 함수 (아이템 정보 반환)
	int GetItemIdFromSlot(int slotIndex) const {
		const ItemSlot& slot = _inventory.GetSlot(slotIndex);
		return slot.IsEmpty() ? 0 : slot.itemId;
	}

private:
	void SaveSlotToDB(int slotIndex) {
		int characterId = static_cast<int>(playerId);
		const ItemSlot& slot = _inventory.GetSlot(slotIndex);
		
		if (slot.IsEmpty()) {
			// 빈 슬롯이면 DB에서 삭제
			InventoryRepository::DeleteInventorySlotAsync(characterId, slotIndex);
		} else {
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
	void SetRoom(const shared_ptr<Room>& r) {room = r;}
	shared_ptr<Room> GetRoom() const		{return room.lock(); }

/*-----------------------
	Party Data
-----------------------*/
public:
	inline int32 GetPartyId() const { return _partyId; }
	inline void SetPartyId(int32 partyId) {_partyId = partyId;}
	inline bool IsInParty() const {return _partyId != 0;}

private:
	int32 _partyId = 0; // 파티 ID (0 = 파티 없음)

};

