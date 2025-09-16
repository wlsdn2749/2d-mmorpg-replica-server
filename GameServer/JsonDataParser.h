#pragma once

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <stdexcept>

/**
 * JSON 데이터 파싱을 위한 기본 유틸리티 클래스
 * Resources/Data 폴더의 JSON 파일들을 파싱하는 공통 기능 제공
 */
class JsonDataParser
{
private:
    /**
     * Resources/Data 폴더의 전체 경로를 반환
     */
    static std::string GetResourcesDataPath();

    /**
     * JSON 파일을 로드하여 rapidjson::Document로 반환
     * @param filename JSON 파일명 (예: "Monster_data.json")
     * @return 파싱된 JSON Document 객체
     * @throws std::runtime_error 파일이 없거나 파싱 실패시
     */
    static rapidjson::Document LoadJsonDocument(const std::string& filename);

public:
    /**
     * JSON 배열 데이터를 벡터로 변환
     * @param filename JSON 파일명
     * @param converter JSON Value를 T 타입으로 변환하는 함수
     * @return 변환된 데이터의 벡터
     */
    template<typename T>
    static std::vector<T> ParseArrayData(const std::string& filename,
                                        std::function<T(const rapidjson::Value&)> converter)
    {
        auto document = LoadJsonDocument(filename);
        std::vector<T> result;

        if (!document.IsArray())
        {
            throw std::runtime_error("JSON 파일이 배열 형식이 아닙니다: " + filename);
        }

        result.reserve(document.Size());
        for (auto& item : document.GetArray())
        {
            result.push_back(converter(item));
        }

        return result;
    }

    /**
     * JSON 배열 데이터를 맵으로 변환
     * @param filename JSON 파일명
     * @param converter JSON Value를 T 타입으로 변환하는 함수
     * @param keyExtractor JSON Value에서 키를 추출하는 함수
     * @return 변환된 데이터의 맵
     */
    template<typename T>
    static std::unordered_map<int, T> ParseMapData(const std::string& filename,
                                                  std::function<T(const rapidjson::Value&)> converter,
                                                  std::function<int(const rapidjson::Value&)> keyExtractor)
    {
        auto document = LoadJsonDocument(filename);
        std::unordered_map<int, T> result;

        if (!document.IsArray())
        {
            throw std::runtime_error("JSON 파일이 배열 형식이 아닙니다: " + filename);
        }

        for (auto& item : document.GetArray())
        {
            int key = keyExtractor(item);
            result[key] = converter(item);
        }

        return result;
    }

    /**
     * JSON Value에서 문자열을 int32_t로 안전하게 변환
     */
    static int32_t SafeGetInt(const rapidjson::Value& value, const std::string& fieldName);

    /**
     * JSON Value에서 문자열을 안전하게 추출
     */
    static std::string SafeGetString(const rapidjson::Value& value, const std::string& fieldName);
};