#include "pch.h"
#include "XmlParser.h"
#include "FileUtils.h"
#include "CoreGlobal.h"

/*-------------
	XmlNode
--------------*/

_locale_t kr = _create_locale(LC_NUMERIC, "kor");

bool XmlNode::GetBoolAttr(const WCHAR* key, bool defaultValue)
{
	XmlAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return ::_wcsicmp(attr->value(), L"true") == 0;

	return defaultValue;
}

int8 XmlNode::GetInt8Attr(const WCHAR* key, int8 defaultValue)
{
	XmlAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return static_cast<int8>(::_wtoi(attr->value()));

	return defaultValue;
}

int16 XmlNode::GetInt16Attr(const WCHAR* key, int16 defaultValue)
{
	XmlAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return static_cast<int16>(::_wtoi(attr->value()));

	return defaultValue;
}

int32 XmlNode::GetInt32Attr(const WCHAR* key, int32 defaultValue)
{
	XmlAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return ::_wtoi(attr->value());

	return defaultValue;
}

int64 XmlNode::GetInt64Attr(const WCHAR* key, int64 defaultValue)
{
	xml_attribute<WCHAR>* attr = _node->first_attribute(key);
	if (attr)
		return ::_wtoi64(attr->value());

	return defaultValue;
}

float XmlNode::GetFloatAttr(const WCHAR* key, float defaultValue)
{
	XmlAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return static_cast<float>(::_wtof(attr->value()));

	return defaultValue;
}

double XmlNode::GetDoubleAttr(const WCHAR* key, double defaultValue)
{
	XmlAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return ::_wtof_l(attr->value(), kr);

	return defaultValue;
}

const WCHAR* XmlNode::GetStringAttr(const WCHAR* key, const WCHAR* defaultValue)
{
	XmlAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return attr->value();

	return defaultValue;
}

bool XmlNode::GetBoolValue(bool defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return ::_wcsicmp(val, L"true") == 0;

	return defaultValue;
}

int8 XmlNode::GetInt8Value(int8 defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<int8>(::_wtoi(val));

	return defaultValue;
}

int16 XmlNode::GetInt16Value(int16 defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<int16>(::_wtoi(val));
	return defaultValue;
}

int32 XmlNode::GetInt32Value(int32 defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<int32>(::_wtoi(val));

	return defaultValue;
}

int64 XmlNode::GetInt64Value(int64 defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<int64>(::_wtoi64(val));

	return defaultValue;
}

float XmlNode::GetFloatValue(float defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<float>(::_wtof(val));

	return defaultValue;
}

double XmlNode::GetDoubleValue(double defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return ::_wtof_l(val, kr);

	return defaultValue;
}

const WCHAR* XmlNode::GetStringValue(const WCHAR* defaultValue)
{
	WCHAR* val = _node->first_node()->value();
	if (val)
		return val;

	return defaultValue;
}

XmlNode XmlNode::FindChild(const WCHAR* key)
{
	return XmlNode(_node->first_node(key));
}

Vector<XmlNode> XmlNode::FindChildren(const WCHAR* key)
{
	Vector<XmlNode> nodes;

	xml_node<WCHAR>* node = _node->first_node(key);
	while (node)
	{
		nodes.push_back(XmlNode(node));
		node = node->next_sibling(key);
	}

	return nodes;
}

/*---------------
	XmlParser
----------------*/

bool XmlParser::ParseFromFile(const WCHAR* path, OUT XmlNode& root)
{
	Vector<BYTE> bytes = FileUtils::ReadFile(path);
	_data = FileUtils::Convert(string(bytes.begin(), bytes.end()));

	if (_data.empty())
		return false;

	_document = MakeShared<XmlDocumentType>();
	_document->parse<0>(reinterpret_cast<WCHAR*>(&_data[0]));
	root = XmlNode(_document->first_node());
	return true;
}