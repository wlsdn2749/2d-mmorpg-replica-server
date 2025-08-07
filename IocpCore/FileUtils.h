#pragma once
#include <vector>
#include "Types.h"

/*----------------------
	FileUtils
----------------------*/

class FileUtils
{
public:
	static Vector<BYTE>	ReadFile(const WCHAR* path);
	static String		Convert(string str);
};

