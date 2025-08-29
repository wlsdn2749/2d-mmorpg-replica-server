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

    class GetCharactersByUser : public DBBind<1,8>
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

    private:
    	int32 _userId = {};
    };

    class UpdateCharacterStats : public DBBind<8,0>
    {
    public:
    	UpdateCharacterStats(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spUpdateCharacterStats(?,?,?,?,?,?,?,?)}") { }
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
    	void ParamIn_Level(int32& v) { BindParam(6, v); };
    	void ParamIn_Level(int32&& v) { _level = std::move(v); BindParam(6, _level); };
    	void ParamIn_Exp(int32& v) { BindParam(7, v); };
    	void ParamIn_Exp(int32&& v) { _exp = std::move(v); BindParam(7, _exp); };

    private:
    	int32 _characterId = {};
    	int32 _posX = {};
    	int32 _posY = {};
    	int32 _dir = {};
    	int32 _lastRoom = {};
    	int32 _hp = {};
    	int32 _level = {};
    	int32 _exp = {};
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


     
};