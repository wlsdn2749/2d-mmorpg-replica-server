#pragma once
#include <future>
#include "DBDisPatcher.h"
#include "GenProcedures.h"

struct CharacterRepository
{
    struct ValidationResult
    {
        bool isValid;
        std::string message;
    };

    struct CharacterInfo
    {
        int characterId;
        string username;
        
        int posX;
        int posY;

        Protocol::EGender gender;
        Protocol::ERegion region;
        Protocol::EDirection dir;

        int level;
    };

/* 캐릭터 생성 요청*/
public:
	static void CreateCharacter_DB(DBConnection& conn, int userId, String username, Protocol::EGender gender, Protocol::ERegion region)
	{
        SP::CreateCharacter sp(conn);
        sp.ParamIn_UserId(userId); // ForeignKey
        sp.ParamIn_Username(username.c_str(), static_cast<int>(username.size()));
        sp.ParamIn_Gender(gender);
        sp.ParamIn_Region(region);
        sp.ParamIn_LastZone(0);
        sp.Execute();
	}

    static std::future<void> CreateCharacterAsync(int userId, wstring_view username, Protocol::EGender gender, Protocol::ERegion region)
    {
        auto w_username = String(username);
        return DbDispatcher::EnqueueRet([userId, w_username, gender, region](DBConnection& c) {
            CreateCharacter_DB(c, userId, w_username, gender, region);
            });
    }
/* 동일 Username 존재 판단*/
public:
	static ValidationResult IsValidUsername(std::string username) // username이 u8문자열
	{
        auto fut = CharacterUsernameExists(username); // 미리 비동기 처리

		// 2-6자, 한글 아니면 바로 리턴
        if (!IsValidKoreanNameAndLengths(username))
        {
            return ValidationResult {false, "이름이 길거나 적합하지 않습니다"};
        }
        
        // DB에 중복 되어있는 문자열 확인
        auto isDuplicated = fut.get();
        if (isDuplicated)
        {
            return ValidationResult {false, "등록된 이름입니다."};
        }

        return ValidationResult {true, ""}; 
	}
    static std::future<bool> CharacterUsernameExists(std::string username)
    {
        return DbDispatcher::EnqueueRet([username](DBConnection& c) {
                return CharacterUsernameExists_DB(c, username);
            });
    }
    static bool CharacterUsernameExists_DB(DBConnection& conn, std::string username)
    {
        int exists;
        String wname = StrToWstr(username);
        SP::CharacterUsernameExists sp(conn);
        sp.ParamIn_Username(wname.c_str(), static_cast<int>(wname.size()));
        sp.ParamOut_Exists(OUT exists);
        sp.Execute();
        if (exists) return true; // 존재하면
        else return false; // 존재하지 않으면
    }
    static bool IsValidKoreanNameAndLengths(const std::string& username)
    {
        // UTF-8을 코드포인트 단위로 파싱
        int count = 0;
        for (size_t i = 0; i < username.size();)
        {
            unsigned char c = username[i];

            // 한글은 UTF-8에서 3바이트
            if (c >= 0xE0 && c <= 0xEF && i + 2 < username.size())
            {
                unsigned int codepoint =
                    ((username[i] & 0x0F) << 12) |
                    ((username[i + 1] & 0x3F) << 6) |
                    (username[i + 2] & 0x3F);

                // 한글 범위 체크
                if (codepoint < 0xAC00 || codepoint > 0xD7A3)
                    return false;

                count++;
                i += 3; // UTF-8 한글은 3바이트
            }
            else
            {
                // 한글이 아닌 다른 문자가 나오면 false
                return false;
            }
        }

        // 2~6자만 허용
        return (count >= 2 && count <= 6);
    }


/* 캐릭터 리스트 받아오기*/
public:
    static Vector<CharacterInfo> GetCharactersByUser_DB(DBConnection& conn, int userId)
    {
        Vector<CharacterInfo> characters;
        int characterId;
        WCHAR username[100];
        int posX;
        int posY;

        int gender;
        int region;
        int dir;
        int level;

        SP::GetCharactersByUser sp(conn);
        sp.ParamIn_UserId(userId);
        sp.ColumnOut_CharacterId(OUT characterId);
        sp.ColumnOut_Username(OUT username);
        sp.ColumnOut_PosX(OUT posX);
        sp.ColumnOut_PosY(OUT posY);
        sp.ColumnOut_Gender(OUT gender);
        sp.ColumnOut_Region(OUT region);
        sp.ColumnOut_Dir(OUT dir);
        sp.ColumnOut_Level(OUT level);
        sp.Execute();
        while (sp.Fetch())
        {
            GConsoleLogger->WriteStdOut(Color::GREEN,
                L"Username[%s] Gender[%d] Region[%d] Level[%d]\n", username, gender, region, level);

            CharacterInfo info
            {
                characterId,
                WstrToStr(username),
                posX,
                posY,
                static_cast<Protocol::EGender>(gender),
                static_cast<Protocol::ERegion>(region),
                static_cast<Protocol::EDirection>(dir),
                level
            };
            characters.push_back(info);
        }

        return characters;
    }
    static std::future<Vector<CharacterInfo>> GetCharactersByUserAsync(int userId)
    {
        return DbDispatcher::EnqueueRet([userId](DBConnection& c) {
            return GetCharactersByUser_DB(c, userId);
            });
    }
};

