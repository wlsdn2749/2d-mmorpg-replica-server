#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

namespace SP
{
	
    class Register : public DBBind<5,0>
    {
    public:
    	Register(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spRegister(?,?,?,?,?)}") { }
    	template<int32 N> void ParamIn_Email(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void ParamIn_Email(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void ParamIn_Email(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void ParamIn_Email(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	template<typename T, int32 N> void ParamIn_PwHash(T(&v)[N]) { BindParam(1, v); };
    	template<typename T> void ParamIn_PwHash(T* v, int32 count) { BindParam(1, v, count); };
    	template<typename T, int32 N> void ParamIn_PwSalt(T(&v)[N]) { BindParam(2, v); };
    	template<typename T> void ParamIn_PwSalt(T* v, int32 count) { BindParam(2, v, count); };
    	void ParamOut_AccId(OUT int32& v) { BindParam(3, v, SQL_PARAM_OUTPUT); };
    	void ParamOut_Result(OUT int32& v) { BindParam(4, v, SQL_PARAM_OUTPUT); };

    private:
    	int32 _accId = {};
    	int32 _result = {};
    };

    class GetAccountHashSalt : public DBBind<1,2>
    {
    public:
    	GetAccountHashSalt(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetAccountHashSalt(?)}") { }
    	template<int32 N> void ParamIn_Email(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void ParamIn_Email(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void ParamIn_Email(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void ParamIn_Email(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	template<typename T, int32 N> void ColumnOut_PwHash(OUT T(&v)[N]) { BindCol(0, v); }
    	template<typename T, size_t N> void ColumnOut_PwHash(OUT std::array<T, N>& v) { BindCol(0, v); } // Array 전용 템플릿
    	template<typename T, int32 N> void ColumnOut_PwSalt(OUT T(&v)[N]) { BindCol(1, v); }
    	template<typename T, size_t N> void ColumnOut_PwSalt(OUT std::array<T, N>& v) { BindCol(1, v); } // Array 전용 템플릿

    private:
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