#pragma once

/*---------------
	ConsoleLog
----------------*/

enum class Color
{
	BLACK,
	WHITE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
};

class ConsoleLogger
{
	enum { BUFFER_SIZE = 4096 };

public:
	ConsoleLogger();
	~ConsoleLogger();

public:
	void		WriteStdOut(Color color, const WCHAR* str, ...);
	void		WriteStdErr(Color color, const WCHAR* str, ...);

protected:
	void		SetColor(bool stdOut, Color color);

private:
	HANDLE		_stdOut;
	HANDLE		_stdErr;
};