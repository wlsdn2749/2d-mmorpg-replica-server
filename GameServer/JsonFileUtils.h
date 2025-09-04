#pragma once

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

using namespace rapidjson;

class JsonFileUtils
{
public:
	template<typename T>
	static std::pair<T, T> parseArray2(const Value& arr)
	{
		if (!arr.IsArray() || arr.Size() < 2)
		{
			throw std::runtime_error("INVALID ARRAY FORMAT");
		}

		return {arr[0].Get<T>(), arr[1].Get<T>()};
	}
};

