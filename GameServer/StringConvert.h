#include <Windows.h>
#include <string>
#include <stdexcept>

/*───────────────────────────────────────────────
 * 1) UTF-8(std::string) → UTF-16(String)
 *──────────────────────────────────────────────*/
inline String StrToWstr(const std::string& src)      // string ➜ String
{
    if (src.empty())
        return {};

    const int len16 = ::MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        src.data(), static_cast<int>(src.size()),
        nullptr, 0);

    if (len16 <= 0)
        throw std::runtime_error("StrToWstr size fail");

    String dst(len16, L'\0');         // 커스텀 allocator 사용
    const int written = ::MultiByteToWideChar(
        CP_UTF8, 0,
        src.data(), static_cast<int>(src.size()),
        dst.data(), len16);

    if (written != len16)
        throw std::runtime_error("StrToWstr conv fail");

    return dst;
}

/*───────────────────────────────────────────────
 * 2) UTF-16(String) → UTF-8(std::string)
 *──────────────────────────────────────────────*/
inline std::string WstrToStr(const String& src)      // String ➜ string
{
    if (src.empty())
        return {};

    const int len8 = ::WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS,
        src.data(), static_cast<int>(src.size()),
        nullptr, 0,
        nullptr, nullptr);

    if (len8 <= 0)
        throw std::runtime_error("WstrToStr size fail");

    std::string dst(len8, '\0');
    const int written = ::WideCharToMultiByte(
        CP_UTF8, 0,
        src.data(), static_cast<int>(src.size()),
        dst.data(), len8,
        nullptr, nullptr);

    if (written != len8)
        throw std::runtime_error("WstrToStr conv fail");

    return dst;
}