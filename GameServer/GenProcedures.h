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
    	void ParamIn_LastZone(int32& v) { BindParam(4, v); };
    	void ParamIn_LastZone(int32&& v) { _lastZone = std::move(v); BindParam(4, _lastZone); };

    private:
    	int32 _userId = {};
    	int32 _gender = {};
    	int32 _region = {};
    	int32 _lastZone = {};
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

    class GetCharactersByUser : public DBBind<1,4>
    {
    public:
    	GetCharactersByUser(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetCharactersByUser(?)}") { }
    	void ParamIn_UserId(int32& v) { BindParam(0, v); };
    	void ParamIn_UserId(int32&& v) { _userId = std::move(v); BindParam(0, _userId); };
    	template<int32 N> void ColumnOut_Username(OUT WCHAR(&v)[N]) { BindCol(0, v); };
    	void ColumnOut_Gender(OUT int32& v) { BindCol(1, v); };
    	void ColumnOut_Region(OUT int32& v) { BindCol(2, v); };
    	void ColumnOut_Level(OUT int32& v) { BindCol(3, v); };

    private:
    	int32 _userId = {};
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