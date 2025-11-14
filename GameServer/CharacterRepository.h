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

    struct CharacterStat
    {
        int characterId;
        int posX;
        int posY;
        Protocol::EDirection dir;
        int lastRoom;
        int hp;
        int maxHp;
        int level;
        int exp;
        int money;
    };

/* 캐릭터 생성 요청*/
public:
	static void CreateCharacter_DB(DBConnection& conn, int userId, String username, Protocol::EGender gender, Protocol::ERegion region, int lastRoom);
    static std::future<void> CreateCharacterAsync(int userId, wstring_view username, Protocol::EGender gender, Protocol::ERegion region, int lastRoom);

/* 동일 Username 존재 판단*/
	static ValidationResult IsValidUsername(std::string username); // username이 u8문자열
    static std::future<bool> CharacterUsernameExists(std::string username);
    static bool CharacterUsernameExists_DB(DBConnection& conn, std::string username);
    static bool IsValidKoreanNameAndLengths(const std::string& username);

/* 캐릭터 리스트 받아오기*/
    static Vector<CharacterInfo> GetCharactersByUser_DB(DBConnection& conn, int userId);
    static std::future<Vector<CharacterInfo>> GetCharactersByUserAsync(int userId);

/* 캐릭터 정보 업데이트*/
    static void UpdateCharacterStats_DB(DBConnection& conn, const CharacterStat& stat);
    static std::future<void> UpdateCharacterStatsAsync(const CharacterStat &stat);

// 캐릭터 정보 받아오기
    static CharacterStat GetCharacterStats_DB(DBConnection& conn, int characterId);
    static std::future<CharacterStat> GetCharacterStatsAsync(int characterId);

/* 캐릭터 삭제 (Soft Delete)*/
    static bool DeleteCharacter_DB(DBConnection& conn, int userId, int characterId);
    static std::future<bool> DeleteCharacterAsync(int userId, int characterId);
};

