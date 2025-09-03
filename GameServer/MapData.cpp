#include "pch.h"
#include "MapData.h"
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include "JsonFileUtils.h"

std::shared_ptr<MapData> MapData::LoadMapFromJsonFile(const std::string& path)
{
    auto mapData = std::make_shared<MapData>();

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open mapfile: " + path);;

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string jsonContent;
    jsonContent.reserve(fileSize);

    // 파일 내용을 string으로 읽기
    jsonContent.assign((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    Document document;
    document.Parse(jsonContent.c_str());

    if (document.HasParseError()) {
        throw std::runtime_error("문자열 파싱 실패 (오류 코드: " +
            std::to_string(document.GetParseError()) + ")");
    }

    // 기본 정보 파싱
    mapData->version = document["Version"].GetInt();

    // cellSize 배열 파싱
    auto cellSize = JsonFileUtils::parseArray2<float>(document["CellSize"]);
    mapData->cellSizeX = cellSize.first;
    mapData->cellSizeY = cellSize.second;

    // origin 배열 파싱
    auto origin = JsonFileUtils::parseArray2<int>(document["Origin"]);
    mapData->originX = origin.first;
    mapData->originY = origin.second;

    mapData->width = document["Width"].GetInt();
    mapData->height = document["Height"].GetInt();

    if (mapData->height <= 0)
        throw std::runtime_error("Empty map data: " + path);

    mapData->tiles.resize(mapData->width * mapData->height);

    const Value& passableRows = document["PassableRowsTopDown"];
    if (passableRows.IsArray())
    {
        if (passableRows.Size() != mapData->height)
        {
            throw std::runtime_error("Irregular row height in map: " + path);
        }

        for (int r = 0; r < passableRows.Size(); ++r)
        {
            const string line = passableRows[r].GetString();
            if (line.length() != mapData->width)
            {
                throw std::runtime_error("Irregular row width in map: " + path);
            }
            for (int c = 0; c < line.length(); ++c)
            {
                const char ch = line[c];
                if(ch != '0' && ch != '1' && ch != '2')
                    throw std::runtime_error("Irregular tile char (expect 0/1/2): " + path);

                mapData->tiles[r * mapData->width + c] = static_cast<uint8_t>(ch - '0');
            }
        }
    }

    const Value& spawns = document["Spawns"];
    if (spawns.IsArray()) 
    {
        for (SizeType i = 0; i < spawns.Size(); i++) 
        {
            const Value& spawn = spawns[i];
            SpawnPoint spawnPoint;
            spawnPoint.spawnType = GetESpawnType(spawn["Type"].GetString());
            spawnPoint.x = spawn["X"].GetInt();
            spawnPoint.y = spawn["Y"].GetInt();
            mapData->spawnPoints.insert({i, spawnPoint});
        }
    }

    const Value& portals = document["Portals"];
    if (portals.IsArray())
    {
        for (SizeType i = 0; i < portals.Size(); i++)
        {
            const Value& portal = portals[i];
            const int srcPortalId = stoi(portal["SrcPortalId"].GetString());
            const int dstMapId = stoi(portal["DstMapId"].GetString());
            const int dstPortalId = stoi(portal["DstPortalId"].GetString());
            const int x = portal["X"].GetInt();
            const int y = portal["Y"].GetInt();
            mapData->DefinePortal(PortalLink {srcPortalId, dstMapId, dstPortalId} );
            mapData->MapTileToPortal(ESpawnType::PORTAL, x, y, srcPortalId);
        }
    }

    // mapOffset 배열 파싱
    //auto mapOffset = JsonFileUtils::parseArray2<int>(document["mapOffset"]);
    //mapData->mapOffsetX = mapOffset.first;
    //mapData->mapOffsetY = mapOffset.second;

    // worldTopLeft 배열 파싱
    auto worldTopLeft = JsonFileUtils::parseArray2<int>(document["WorldTopLeft"]);
    mapData->worldTopLeftX = worldTopLeft.first;
    mapData->worldTopLeftY = worldTopLeft.second;
    mapData->minX = mapData->worldTopLeftX;
    mapData->maxY = mapData->worldTopLeftY;

    // worldBottomRight 배열 파싱
    auto worldBottomRight = JsonFileUtils::parseArray2<int>(document["WorldBottomRight"]);
    mapData->worldBottomRightX = worldBottomRight.first;
    mapData->worldBottomRightY = worldBottomRight.second;
    mapData->maxX = mapData->worldBottomRightX;
    mapData->minY = mapData->worldBottomRightY;

    return mapData;
}

ESpawnType MapData::GetESpawnType(string type)
{
    if(type == "Player") return ESpawnType::PLAYER_SPAWN;
    if(type == "Portal") return ESpawnType::PORTAL;
}
