#include "pch.h"
#include "JsonDataParser.h"
#include <iostream>

std::string JsonDataParser::GetResourcesDataPath()
{
    // GameServer 실행 파일과 같은 경로의 Resources/Data 폴더
    return "Resources/Data/";
}

rapidjson::Document JsonDataParser::LoadJsonDocument(const std::string& filename)
{
    std::string filePath = GetResourcesDataPath() + filename;

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("JSON 파일을 열 수 없습니다: " + filePath);
    }

    // 파일 크기 확인
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize == 0)
    {
        throw std::runtime_error("JSON 파일이 비어있습니다: " + filePath);
    }

    // 파일 내용을 string으로 읽기
    std::string jsonContent;
    jsonContent.reserve(fileSize);
    jsonContent.assign((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    file.close();

    // JSON 파싱
    rapidjson::Document document;
    document.Parse(jsonContent.c_str());

    if (document.HasParseError())
    {
        throw std::runtime_error("JSON 파싱 실패 (파일: " + filePath +
                               ", 오류 코드: " + std::to_string(document.GetParseError()) + ")");
    }

    return document;
}

int32_t JsonDataParser::SafeGetInt(const rapidjson::Value& value, const std::string& fieldName)
{
    if (!value.HasMember(fieldName.c_str()))
    {
        throw std::runtime_error("JSON에서 필수 필드를 찾을 수 없습니다: " + fieldName);
    }

    const auto& field = value[fieldName.c_str()];

    // 숫자 타입인 경우
    if (field.IsInt())
    {
        return field.GetInt();
    }
    // 문자열 타입인 경우 (Google Sheets에서 오는 데이터는 보통 문자열)
    else if (field.IsString())
    {
        try
        {
            std::string str = field.GetString();
            return std::stoi(str);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("숫자 변환 실패 (필드: " + fieldName + ", 값: " + field.GetString() + ")");
        }
    }
    else
    {
        throw std::runtime_error("잘못된 데이터 타입 (필드: " + fieldName + ")");
    }
}

std::string JsonDataParser::SafeGetString(const rapidjson::Value& value, const std::string& fieldName)
{
    if (!value.HasMember(fieldName.c_str()))
    {
        throw std::runtime_error("JSON에서 필수 필드를 찾을 수 없습니다: " + fieldName);
    }

    const auto& field = value[fieldName.c_str()];

    if (!field.IsString())
    {
        throw std::runtime_error("잘못된 데이터 타입 (필드: " + fieldName + ", 문자열이 아님)");
    }

    return field.GetString();
}

bool JsonDataParser::SafeGetBool(const rapidjson::Value& value, const std::string& fieldName)
{
    if (!value.HasMember(fieldName.c_str()))
    {
        throw std::runtime_error("JSON에서 필수 필드를 찾을 수 없습니다. " + fieldName);
    }

    const auto& field = value[fieldName.c_str()];

    if (!field.IsString())
    {
        throw std::runtime_error("잘못된 데이터 타입 (필드: " + fieldName + ", 문자열이 아님)");
    }

    auto fieldStr = static_cast<string>(field.GetString());

    if(fieldStr == "TRUE") 
        return true;
    else if(fieldStr == "FALSE") 
        return false;
    else 
        throw std::runtime_error("TRUE혹은 FASLE값이 아님 :" + string(fieldStr));
}
