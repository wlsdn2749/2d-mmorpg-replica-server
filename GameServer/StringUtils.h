#include <vector>
#include <string>

namespace StringUtils
{
	// O(n): Split -> vector<string>
	static std::vector<std::string> split(std::string input, std::string delimiter)
	{
		std::vector<std::string> ret;
		long long pos = 0;
		std::string token = "";

		auto s = input.length();
		while ((pos = input.find(delimiter)) != std::string::npos)
		{
			token = input.substr(0, pos);
			ret.push_back(token);
			input.erase(0, pos + delimiter.length());
		}
		ret.push_back(input);
		return ret;
	}
}