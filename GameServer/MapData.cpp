#include "pch.h"
#include "MapData.h"

#include <fstream>

std::shared_ptr<const MapData> MapData::FromFile(const std::string& path)
{
    auto map = std::make_shared<MapData>();

    std::ifstream in(path);
    if (!in.is_open())
        throw std::runtime_error("Failed to open map file: " + path);

    in >> map->minX >> map->maxX >> map->minY >> map->maxY;

    // 남은 줄은 0/1 문자열
    std::string line;
    std::vector<std::string> lines;
    lines.reserve(256);

    while (in >> line)
        lines.push_back(std::move(line));

    map->height = static_cast<int>(lines.size());
    if (map->height <= 0)
        throw std::runtime_error("Empty map data: " + path);

    map->width = static_cast<int>(lines[0].size());
    map->tiles.resize(map->width * map->height);

    for (int r = 0; r < map->height; ++r)
    {
        if ((int)lines[r].size() != map->width)
            throw std::runtime_error("Irregular row width in map: " + path);

        for (int c = 0; c < map->width; ++c)
        {
            const char ch = lines[r][c];
            if (ch != '0' && ch != '1')
                throw std::runtime_error("Invalid tile char (expect 0/1): " + path);

            map->tiles[r * map->width + c] = static_cast<uint8_t>(ch - '0');
        }
    }

    // 경계와 실제 배열 크기가 일치하는지(디버그용) 간단 검증
    const int expectW = map->maxX - map->minX + 1;
    const int expectH = map->maxY - map->minY + 1;
    if (expectW != map->width || expectH != map->height)
    {
        // 치명은 아니고 경고로 볼 수도 있음. 일단 예외로 막아두면 디버깅 편함.
        throw std::runtime_error("Bounds/array size mismatch in map: " + path);
    }

    return map;
}