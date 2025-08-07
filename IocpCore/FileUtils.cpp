#include "pch.h"
#include "FileUtils.h"
#include <filesystem>
#include <fstream>

/*----------------------
	FileUtils
----------------------*/

namespace fs = std::filesystem;

Vector<BYTE> FileUtils::ReadFile(const WCHAR* path)
{
	Vector<BYTE> ret;

	fs::path filePath { path };

	const uint32 fileSize = static_cast<uint32>(fs::file_size(filePath));
	ret.resize(fileSize);

	basic_ifstream<BYTE> inputStream { filePath };
	inputStream.read(&ret[0], fileSize);

	return ret;

}

String FileUtils::Convert(string str)
{
	const int32 srcLen = static_cast<int32>(str.size());

	String ret;
	if (srcLen == 0)
		return ret;

	// 변환 후 길이 구하기
	const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, nullptr, 0);

	// 크기만큼 공간확보
	ret.resize(retLen);

	// 실제 변환 수행
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);
	return ret;
}
