#include "pch.h"
#include "ConsoleLogger.h"
#include <Windows.h>
#include <cwchar>

ConsoleLogger::ConsoleLogger()
{
    _stdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    _stdErr = ::GetStdHandle(STD_ERROR_HANDLE);
}

ConsoleLogger::~ConsoleLogger()
{
}

void ConsoleLogger::WriteStdOut(Color color, const WCHAR* format, ...)
{
    if (!format) return;
    SetColor(true, color);

    WCHAR buffer[BUFFER_SIZE];
    va_list ap;
    va_start(ap, format);
    vswprintf_s(buffer, BUFFER_SIZE, format, ap);
    va_end(ap);

    DWORD written = 0;
    WriteConsoleW(_stdOut, buffer, static_cast<DWORD>(wcslen(buffer)), &written, nullptr);

    SetColor(true, Color::WHITE);
}

void ConsoleLogger::WriteStdErr(Color color, const WCHAR* format, ...)
{
    if (!format) return;
    SetColor(false, color);

    WCHAR buffer[BUFFER_SIZE];
    va_list ap;
    va_start(ap, format);
    vswprintf_s(buffer, BUFFER_SIZE, format, ap);
    va_end(ap);

    DWORD written = 0;
    WriteConsoleW(_stdErr, buffer, static_cast<DWORD>(wcslen(buffer)), &written, nullptr);

    SetColor(false, Color::WHITE);
}

void ConsoleLogger::SetColor(bool stdOut, Color color)
{
    static WORD SColors[] = {
        0,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        FOREGROUND_RED | FOREGROUND_INTENSITY,
        FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
    };

    ::SetConsoleTextAttribute(stdOut ? _stdOut : _stdErr,
        SColors[static_cast<int32>(color)]);
}
