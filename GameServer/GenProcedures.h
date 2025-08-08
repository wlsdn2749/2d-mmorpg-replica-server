#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

namespace SP
{
	
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