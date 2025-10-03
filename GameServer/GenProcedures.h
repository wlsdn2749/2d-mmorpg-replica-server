#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

namespace SP
{
	
    class AccountsUpsert : public DBBind<1,0>
    {
    public:
    	AccountsUpsert(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spAccountsUpsert(?)}") { }
    	void ParamIn_UserId(int32& v) { BindParam(0, v); };
    	void ParamIn_UserId(int32&& v) { _userId = std::move(v); BindParam(0, _userId); };

    private:
    	int32 _userId = {};
    };

    class CreateCharacter : public DBBind<5,0>
    {
    public:
    	CreateCharacter(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spCreateCharacter(?,?,?,?,?)}") { }
    	void ParamIn_UserId(int32& v) { BindParam(0, v); };
    	void ParamIn_UserId(int32&& v) { _userId = std::move(v); BindParam(0, _userId); };
    	template<int32 N> void ParamIn_Username(WCHAR(&v)[N]) { BindParam(1, v); };
    	template<int32 N> void ParamIn_Username(const WCHAR(&v)[N]) { BindParam(1, v); };
    	void ParamIn_Username(WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void ParamIn_Username(const WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void ParamIn_Gender(int32& v) { BindParam(2, v); };
    	void ParamIn_Gender(int32&& v) { _gender = std::move(v); BindParam(2, _gender); };
    	void ParamIn_Region(int32& v) { BindParam(3, v); };
    	void ParamIn_Region(int32&& v) { _region = std::move(v); BindParam(3, _region); };
    	void ParamIn_LastRoom(int32& v) { BindParam(4, v); };
    	void ParamIn_LastRoom(int32&& v) { _lastRoom = std::move(v); BindParam(4, _lastRoom); };

    private:
    	int32 _userId = {};
    	int32 _gender = {};
    	int32 _region = {};
    	int32 _lastRoom = {};
    };

    class CharacterUsernameExists : public DBBind<2,0>
    {
    public:
    	CharacterUsernameExists(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spCharacterUsernameExists(?,?)}") { }
    	template<int32 N> void ParamIn_Username(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void ParamIn_Username(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void ParamIn_Username(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void ParamIn_Username(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void ParamOut_Exists(OUT int32& v) { BindParam(1, v, SQL_PARAM_OUTPUT); };

    private:
    	int32 _exists = {};
    };

    class GetCharactersByUser : public DBBind<1,9>
    {
    public:
    	GetCharactersByUser(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetCharactersByUser(?)}") { }
    	void ParamIn_UserId(int32& v) { BindParam(0, v); };
    	void ParamIn_UserId(int32&& v) { _userId = std::move(v); BindParam(0, _userId); };
    	void ColumnOut_CharacterId(OUT int32& v) { BindCol(0, v); };
    	template<int32 N> void ColumnOut_Username(OUT WCHAR(&v)[N]) { BindCol(1, v); };
    	void ColumnOut_PosX(OUT int32& v) { BindCol(2, v); };
    	void ColumnOut_PosY(OUT int32& v) { BindCol(3, v); };
    	void ColumnOut_Gender(OUT int32& v) { BindCol(4, v); };
    	void ColumnOut_Region(OUT int32& v) { BindCol(5, v); };
    	void ColumnOut_Dir(OUT int32& v) { BindCol(6, v); };
    	void ColumnOut_Level(OUT int32& v) { BindCol(7, v); };
    	void ColumnOut_Money(OUT int32& v) { BindCol(8, v); };

    private:
    	int32 _userId = {};
    };

    class UpdateCharacterStats : public DBBind<10,0>
    {
    public:
    	UpdateCharacterStats(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spUpdateCharacterStats(?,?,?,?,?,?,?,?,?,?)}") { }
    	void ParamIn_CharacterId(int32& v) { BindParam(0, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(0, _characterId); };
    	void ParamIn_PosX(int32& v) { BindParam(1, v); };
    	void ParamIn_PosX(int32&& v) { _posX = std::move(v); BindParam(1, _posX); };
    	void ParamIn_PosY(int32& v) { BindParam(2, v); };
    	void ParamIn_PosY(int32&& v) { _posY = std::move(v); BindParam(2, _posY); };
    	void ParamIn_Dir(int32& v) { BindParam(3, v); };
    	void ParamIn_Dir(int32&& v) { _dir = std::move(v); BindParam(3, _dir); };
    	void ParamIn_LastRoom(int32& v) { BindParam(4, v); };
    	void ParamIn_LastRoom(int32&& v) { _lastRoom = std::move(v); BindParam(4, _lastRoom); };
    	void ParamIn_Hp(int32& v) { BindParam(5, v); };
    	void ParamIn_Hp(int32&& v) { _hp = std::move(v); BindParam(5, _hp); };
    	void ParamIn_MaxHp(int32& v) { BindParam(6, v); };
    	void ParamIn_MaxHp(int32&& v) { _maxHp = std::move(v); BindParam(6, _maxHp); };
    	void ParamIn_Level(int32& v) { BindParam(7, v); };
    	void ParamIn_Level(int32&& v) { _level = std::move(v); BindParam(7, _level); };
    	void ParamIn_Exp(int32& v) { BindParam(8, v); };
    	void ParamIn_Exp(int32&& v) { _exp = std::move(v); BindParam(8, _exp); };
    	void ParamIn_Money(int32& v) { BindParam(9, v); };
    	void ParamIn_Money(int32&& v) { _money = std::move(v); BindParam(9, _money); };

    private:
    	int32 _characterId = {};
    	int32 _posX = {};
    	int32 _posY = {};
    	int32 _dir = {};
    	int32 _lastRoom = {};
    	int32 _hp = {};
    	int32 _maxHp = {};
    	int32 _level = {};
    	int32 _exp = {};
    	int32 _money = {};
    };

    class GetCharacterStats : public DBBind<1,9>
    {
    public:
    	GetCharacterStats(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetCharacterStats(?)}") { }
    	void ParamIn_CharacterId(int32& v) { BindParam(0, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(0, _characterId); };
    	void ColumnOut_PosX(OUT int32& v) { BindCol(0, v); };
    	void ColumnOut_PosY(OUT int32& v) { BindCol(1, v); };
    	void ColumnOut_Dir(OUT int32& v) { BindCol(2, v); };
    	void ColumnOut_LastRoom(OUT int32& v) { BindCol(3, v); };
    	void ColumnOut_Hp(OUT int32& v) { BindCol(4, v); };
    	void ColumnOut_MaxHp(OUT int32& v) { BindCol(5, v); };
    	void ColumnOut_Level(OUT int32& v) { BindCol(6, v); };
    	void ColumnOut_Exp(OUT int32& v) { BindCol(7, v); };
    	void ColumnOut_Money(OUT int32& v) { BindCol(8, v); };

    private:
    	int32 _characterId = {};
    };

    class DeleteCharacter : public DBBind<2,1>
    {
    public:
    	DeleteCharacter(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spDeleteCharacter(?,?)}") { }
    	void ParamIn_UserId(int32& v) { BindParam(0, v); };
    	void ParamIn_UserId(int32&& v) { _userId = std::move(v); BindParam(0, _userId); };
    	void ParamIn_CharacterId(int32& v) { BindParam(1, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(1, _characterId); };
    	void ColumnOut_RowCount(OUT int64& v) { BindCol(0, v); };

    private:
    	int32 _userId = {};
    	int32 _characterId = {};
    };

    class GetMonsterDropItems : public DBBind<0,7>
    {
    public:
    	GetMonsterDropItems(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetMonsterDropItems}") { }
    	void ColumnOut_Id(OUT int32& v) { BindCol(0, v); };
    	void ColumnOut_MonsterId(OUT int32& v) { BindCol(1, v); };
    	void ColumnOut_ItemId(OUT int32& v) { BindCol(2, v); };
    	void ColumnOut_DropRate(OUT int32& v) { BindCol(3, v); };
    	void ColumnOut_MinCount(OUT int32& v) { BindCol(4, v); };
    	void ColumnOut_MaxCount(OUT int32& v) { BindCol(5, v); };
    	void ColumnOut_IsEnabled(OUT int32& v) { BindCol(6, v); };

    private:
    };

    class GetEquipmentInstance : public DBBind<1,4>
    {
    public:
    	GetEquipmentInstance(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetEquipmentInstance(?)}") { }
    	void ParamIn_EquipmentInstanceId(int32& v) { BindParam(0, v); };
    	void ParamIn_EquipmentInstanceId(int32&& v) { _equipmentInstanceId = std::move(v); BindParam(0, _equipmentInstanceId); };
    	void ColumnOut_EquipmentInstanceId(OUT int32& v) { BindCol(0, v); };
    	void ColumnOut_ItemId(OUT int32& v) { BindCol(1, v); };
    	void ColumnOut_EnhancementLevel(OUT int32& v) { BindCol(2, v); };
    	void ColumnOut_AcquiredAt(OUT TIMESTAMP_STRUCT& v) { BindCol(3, v); };

    private:
    	int32 _equipmentInstanceId = {};
    };

    class InsertEquipmentInstance : public DBBind<3,0>
    {
    public:
    	InsertEquipmentInstance(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertEquipmentInstance(?,?,?)}") { }
    	void ParamIn_ItemId(int32& v) { BindParam(0, v); };
    	void ParamIn_ItemId(int32&& v) { _itemId = std::move(v); BindParam(0, _itemId); };
    	void ParamIn_EnhancementLevel(int32& v) { BindParam(1, v); };
    	void ParamIn_EnhancementLevel(int32&& v) { _enhancementLevel = std::move(v); BindParam(1, _enhancementLevel); };
    	void ParamOut_NewEquipmentInstanceId(OUT int32& v) { BindParam(2, v, SQL_PARAM_OUTPUT); };

    private:
    	int32 _itemId = {};
    	int32 _enhancementLevel = {};
    	int32 _newEquipmentInstanceId = {};
    };

    class UpdateEquipmentInstance : public DBBind<2,0>
    {
    public:
    	UpdateEquipmentInstance(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spUpdateEquipmentInstance(?,?)}") { }
    	void ParamIn_EquipmentInstanceId(int32& v) { BindParam(0, v); };
    	void ParamIn_EquipmentInstanceId(int32&& v) { _equipmentInstanceId = std::move(v); BindParam(0, _equipmentInstanceId); };
    	void ParamIn_EnhancementLevel(int32& v) { BindParam(1, v); };
    	void ParamIn_EnhancementLevel(int32&& v) { _enhancementLevel = std::move(v); BindParam(1, _enhancementLevel); };

    private:
    	int32 _equipmentInstanceId = {};
    	int32 _enhancementLevel = {};
    };

    class DeleteEquipmentInstance : public DBBind<1,0>
    {
    public:
    	DeleteEquipmentInstance(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spDeleteEquipmentInstance(?)}") { }
    	void ParamIn_EquipmentInstanceId(int32& v) { BindParam(0, v); };
    	void ParamIn_EquipmentInstanceId(int32&& v) { _equipmentInstanceId = std::move(v); BindParam(0, _equipmentInstanceId); };

    private:
    	int32 _equipmentInstanceId = {};
    };

    class GetCharacterEquipment : public DBBind<1,3>
    {
    public:
    	GetCharacterEquipment(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetCharacterEquipment(?)}") { }
    	void ParamIn_CharacterId(int32& v) { BindParam(0, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(0, _characterId); };
    	void ColumnOut_CharacterId(OUT int32& v) { BindCol(0, v); };
    	void ColumnOut_SlotType(OUT int32& v) { BindCol(1, v); };
    	void ColumnOut_EquipmentInstanceId(OUT int32& v) { BindCol(2, v); };

    private:
    	int32 _characterId = {};
    };

    class UpsertCharacterEquipment : public DBBind<3,0>
    {
    public:
    	UpsertCharacterEquipment(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spUpsertCharacterEquipment(?,?,?)}") { }
    	void ParamIn_CharacterId(int32& v) { BindParam(0, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(0, _characterId); };
    	void ParamIn_SlotType(int32& v) { BindParam(1, v); };
    	void ParamIn_SlotType(int32&& v) { _slotType = std::move(v); BindParam(1, _slotType); };
    	void ParamIn_EquipmentInstanceId(int32& v) { BindParam(2, v); };
    	void ParamIn_EquipmentInstanceId(int32&& v) { _equipmentInstanceId = std::move(v); BindParam(2, _equipmentInstanceId); };

    private:
    	int32 _characterId = {};
    	int32 _slotType = {};
    	int32 _equipmentInstanceId = {};
    };

    class DeleteCharacterEquipment : public DBBind<2,0>
    {
    public:
    	DeleteCharacterEquipment(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spDeleteCharacterEquipment(?,?)}") { }
    	void ParamIn_CharacterId(int32& v) { BindParam(0, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(0, _characterId); };
    	void ParamIn_SlotType(int32& v) { BindParam(1, v); };
    	void ParamIn_SlotType(int32&& v) { _slotType = std::move(v); BindParam(1, _slotType); };

    private:
    	int32 _characterId = {};
    	int32 _slotType = {};
    };

    class InsertGold : public DBBind<3,0>
    {
    public:
    	InsertGold(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertGold(?,?,?)}") { }
    	void ParamIn_Gold(int32& v) { BindParam(0, v); };
    	void ParamIn_Gold(int32&& v) { _gold = std::move(v); BindParam(0, _gold); };
    	template<int32 N> void ParamIn_Name(WCHAR(&v)[N]) { BindParam(1, v); };
    	template<int32 N> void ParamIn_Name(const WCHAR(&v)[N]) { BindParam(1, v); };
    	void ParamIn_Name(WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void ParamIn_Name(const WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void ParamIn_CreateDate(TIMESTAMP_STRUCT& v) { BindParam(2, v); };
    	void ParamIn_CreateDate(TIMESTAMP_STRUCT&& v) { _createDate = std::move(v); BindParam(2, _createDate); };

    private:
    	int32 _gold = {};
    	TIMESTAMP_STRUCT _createDate = {};
    };

    class GetGold : public DBBind<1,4>
    {
    public:
    	GetGold(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetGold(?)}") { }
    	void ParamIn_Gold(int32& v) { BindParam(0, v); };
    	void ParamIn_Gold(int32&& v) { _gold = std::move(v); BindParam(0, _gold); };
    	void ColumnOut_Id(OUT int32& v) { BindCol(0, v); };
    	void ColumnOut_Gold(OUT int32& v) { BindCol(1, v); };
    	template<int32 N> void ColumnOut_Name(OUT WCHAR(&v)[N]) { BindCol(2, v); };
    	void ColumnOut_CreateDate(OUT TIMESTAMP_STRUCT& v) { BindCol(3, v); };

    private:
    	int32 _gold = {};
    };

    class GetCharacterInventory : public DBBind<1,4>
    {
    public:
    	GetCharacterInventory(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetCharacterInventory(?)}") { }
    	void ParamIn_CharacterId(int32& v) { BindParam(0, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(0, _characterId); };
    	void ColumnOut_SlotIndex(OUT int32& v) { BindCol(0, v); };
    	void ColumnOut_ItemId(OUT int32& v) { BindCol(1, v); };
    	void ColumnOut_Count(OUT int32& v) { BindCol(2, v); };
    	void ColumnOut_IsQuickslot(OUT int32& v) { BindCol(3, v); };

    private:
    	int32 _characterId = {};
    };

    class SaveInventorySlot : public DBBind<5,0>
    {
    public:
    	SaveInventorySlot(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSaveInventorySlot(?,?,?,?,?)}") { }
    	void ParamIn_CharacterId(int32& v) { BindParam(0, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(0, _characterId); };
    	void ParamIn_SlotIndex(int32& v) { BindParam(1, v); };
    	void ParamIn_SlotIndex(int32&& v) { _slotIndex = std::move(v); BindParam(1, _slotIndex); };
    	void ParamIn_ItemId(int32& v) { BindParam(2, v); };
    	void ParamIn_ItemId(int32&& v) { _itemId = std::move(v); BindParam(2, _itemId); };
    	void ParamIn_Count(int32& v) { BindParam(3, v); };
    	void ParamIn_Count(int32&& v) { _count = std::move(v); BindParam(3, _count); };
    	void ParamIn_IsQuickslot(int32& v) { BindParam(4, v); };
    	void ParamIn_IsQuickslot(int32&& v) { _isQuickslot = std::move(v); BindParam(4, _isQuickslot); };

    private:
    	int32 _characterId = {};
    	int32 _slotIndex = {};
    	int32 _itemId = {};
    	int32 _count = {};
    	int32 _isQuickslot = {};
    };

    class DeleteInventorySlot : public DBBind<2,0>
    {
    public:
    	DeleteInventorySlot(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spDeleteInventorySlot(?,?)}") { }
    	void ParamIn_CharacterId(int32& v) { BindParam(0, v); };
    	void ParamIn_CharacterId(int32&& v) { _characterId = std::move(v); BindParam(0, _characterId); };
    	void ParamIn_SlotIndex(int32& v) { BindParam(1, v); };
    	void ParamIn_SlotIndex(int32&& v) { _slotIndex = std::move(v); BindParam(1, _slotIndex); };

    private:
    	int32 _characterId = {};
    	int32 _slotIndex = {};
    };


     
};