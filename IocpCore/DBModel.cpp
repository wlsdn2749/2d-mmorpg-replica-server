#include "pch.h"
#include "DBModel.h"
using namespace DBModel;
#include <regex>

/*-------------
	Column
--------------*/


// Column을 생성하기 위한 Text를 만들어준다.
// CREATE TABLE ....
String Column::CreateText()
{
	return DBModel::Helpers::Format(
		L"[%s] %s %s %s",
		_name.c_str(),
		_typeText.c_str(),
		_nullable ? L"NULL" : L"NOT NULL",
		_identity ? DBModel::Helpers::Format(L"IDENTITY(%d, %d)", _seedValue, _incrementValue).c_str() : L"");
}

/*-----------
	Index
------------*/

String Index::GetUniqueName()
{
	String ret;

	ret += _primaryKey ? L"PK " : L" ";
	ret += _uniqueConstraint ? L"UK " : L" ";
	ret += (_type == IndexType::Clustered ? L"C " : L"NC ");

	for (const ColumnRef& column : _columns)
	{
		ret += L"*";
		ret += column->_name;
		ret += L" ";
	}

	return ret;
}

String Index::CreateName(const String& tableName)
{
	String ret = L"IX_" + tableName;

	for (const ColumnRef& column : _columns)
	{
		ret += L"_";
		ret += column->_name;
	}

	return ret;
}

String Index::GetTypeText()
{
	return (_type == IndexType::Clustered ? L"CLUSTERED" : L"NONCLUSTERED");
}

String Index::GetKeyText()
{
	if (_primaryKey)
		return L"PRIMARY KEY";

	if (_uniqueConstraint)
		return L"UNIQUE";

	return L"";
}

String Index::CreateColumnsText()
{
	String ret;

	const int32 size = static_cast<int32>(_columns.size());
	for (int32 i = 0; i < size; i++)
	{
		if (i > 0)
			ret += L", ";

		ret += DBModel::Helpers::Format(L"[%s]", _columns[i]->_name.c_str());
	}

	return ret;
}

bool Index::DependsOn(const String& columnName)
{
	auto findIt = std::find_if(_columns.begin(), _columns.end(),
		[&](const ColumnRef& column) { return column->_name == columnName; });

	return findIt != _columns.end();
}

/*-----------
	Table
------------*/

ColumnRef Table::FindColumn(const String& columnName)
{
	auto findIt = std::find_if(_columns.begin(), _columns.end(),
		[&](const ColumnRef& column) { return column->_name == columnName; });

	if (findIt != _columns.end())
		return *findIt;

	return nullptr;
}

/*----------------
	Procedures
-----------------*/

String Procedure::GenerateCreateQuery()
{
	const WCHAR* query = L"CREATE PROCEDURE [dbo].[%s] %s AS BEGIN %s END";

	String paramString = GenerateParamString();
	return DBModel::Helpers::Format(query, _name.c_str(), paramString.c_str(), _body.c_str());
}

String Procedure::GenerateAlterQuery()
{
	const WCHAR* query = L"ALTER PROCEDURE [dbo].[%s] %s AS	BEGIN %s END";

	String paramString = GenerateParamString();
	return DBModel::Helpers::Format(query, _name.c_str(), paramString.c_str(), _body.c_str());
}

String Procedure::GenerateParamString()
{
	String str;

	const int32 size = static_cast<int32>(_parameters.size());
	for (int32 i = 0; i < size; i++)
	{
		const DBModel::Param& prm = _parameters[i];
		const WCHAR* outKw = prm._output? L" OUTPUT" : L"";

		// dir="out|inout" -> "OUTPUT" 붙이기
		if (i < size - 1)
			str += DBModel::Helpers::Format(L"\t%s %s %s,\n", prm._name.c_str(), _parameters[i]._type.c_str(), outKw);
		else
			str += DBModel::Helpers::Format(L"\t%s %s %s", prm._name.c_str(), _parameters[i]._type.c_str(), outKw);
	}

	return str;
}


/*-------------
	Helpers
--------------*/

String Helpers::Format(const WCHAR* format, ...)
{
	WCHAR buf[4096];

	va_list ap;
	va_start(ap, format);
	::vswprintf_s(buf, 4096, format, ap);
	va_end(ap);

	return String(buf);
}

String Helpers::DataType2String(DataType type)
{
	switch (type)
	{
	case DataType::TinyInt:		return L"TinyInt";
	case DataType::SmallInt:	return L"SmallInt";
	case DataType::Int:			return L"Int";
	case DataType::Real:		return L"Real";
	case DataType::DateTime:	return L"DateTime";
	case DataType::Float:		return L"Float";
	case DataType::Bit:			return L"Bit";
	case DataType::Numeric:		return L"Numeric";
	case DataType::BigInt:		return L"BigInt";
	case DataType::VarBinary:	return L"VarBinary";
	case DataType::Varchar:		return L"Varchar";
	case DataType::Binary:		return L"Binary";
	case DataType::NVarChar:	return L"NVarChar";
	default:					return L"None";
	}
}

String Helpers::RemoveWhiteSpace(const String& str)
{
	String ret = str;

	ret.erase(
		std::remove_if(ret.begin(), ret.end(), [=](WCHAR ch) { return ::isspace(ch); }),
		ret.end());

	return ret;
}

DataType Helpers::String2DataType(const WCHAR* str, OUT int32& maxLen)
{
	std::wregex reg(L"([a-z]+)(\\((max|\\d+)\\))?");
	std::wcmatch ret;

	if (std::regex_match(str, OUT ret, reg) == false)
		return DataType::None;

	if (ret[3].matched)
		maxLen = ::_wcsicmp(ret[3].str().c_str(), L"max") == 0 ? -1 : _wtoi(ret[3].str().c_str());
	else
		maxLen = 0;

	if (::_wcsicmp(ret[1].str().c_str(), L"TinyInt") == 0) return DataType::TinyInt;
	if (::_wcsicmp(ret[1].str().c_str(), L"SmallInt") == 0) return DataType::SmallInt;
	if (::_wcsicmp(ret[1].str().c_str(), L"Int") == 0) return DataType::Int;
	if (::_wcsicmp(ret[1].str().c_str(), L"Real") == 0) return DataType::Real;
	if (::_wcsicmp(ret[1].str().c_str(), L"DateTime") == 0) return DataType::DateTime;
	if (::_wcsicmp(ret[1].str().c_str(), L"Float") == 0) return DataType::Float;
	if (::_wcsicmp(ret[1].str().c_str(), L"Bit") == 0) return DataType::Bit;
	if (::_wcsicmp(ret[1].str().c_str(), L"Numeric") == 0) return DataType::Numeric;
	if (::_wcsicmp(ret[1].str().c_str(), L"BigInt") == 0) return DataType::BigInt;
	if (::_wcsicmp(ret[1].str().c_str(), L"VarBinary") == 0) return DataType::VarBinary;
	if (::_wcsicmp(ret[1].str().c_str(), L"Varchar") == 0) return DataType::Varchar;
	if (::_wcsicmp(ret[1].str().c_str(), L"Binary") == 0) return DataType::Binary;
	if (::_wcsicmp(ret[1].str().c_str(), L"NVarChar") == 0) return DataType::NVarChar;

	return DataType::None;
}