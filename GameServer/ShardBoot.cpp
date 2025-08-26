#include "pch.h"
#include "ShardBoot.h"

#include "GlobalQueue.h"

#include "SharedOwner.h"
#include "ThreadManager.h"
#include "Room.h"
#include "TownRoom.h"
#include "FieldRoom.h"
#include "RoomManager.h"


static vector<shared_ptr<GlobalQueue>> gQueues; // 샤드 큐들

inline int OwnerIndexForRoom(int roomId, int roomsPerQueue) {
    return (roomId / roomsPerQueue); // ex) 2개씩 묶기
}

void StartShardedQueues(int totalRooms, int roomsPerQueue, uint32 timeBudgetMs)
{
    const int numQueues = (totalRooms + roomsPerQueue - 1) / roomsPerQueue;
    gQueues.reserve(numQueues);

    for (int i = 0; i < numQueues; ++i) {
        auto q = make_shared<GlobalQueue>();
        gQueues.push_back(q);

        GThreadManager->Launch([q, timeBudgetMs] {
            while (true) {
                // 타이머 만료 분배
                GThreadManager->DistributeReservedJobs();
                // 이번 사이클 예산 설정
                LEndTickCount = ::GetTickCount64() + timeBudgetMs;
                // 이 큐만 처리
                GThreadManager->DoGlobalQueueWork(q.get());
            }
            });
    }
}

void CreateRooms()
{
    const int roomsPerQueue = 2;
    // 마을(0)
    {
        auto town0Map = MapData::FromFile("./Resources/Maps/Map_001.txt");
        town0Map->DefinePortal(PortalLink{
            1001, /* srcPortalId */
            1, /* dstMapId */
            2001, /*dstPortalId */
            });
        town0Map->MapTileToPortal(2, 2, 1001);
        auto town0 = std::make_shared<TownRoom>(Room::Cfg{ 0, "Town", 50, 200 }, town0Map);
        
        town0->SetOwner(gQueues[OwnerIndexForRoom(town0->RoomId(), roomsPerQueue)].get());
        town0->StartTicking(); // 매 틱마다 호출하는 함수
        RoomManager::Instance().Add(town0);
    }

    // 사냥터(0)
    {
        auto field0Map = MapData::FromFile("./Resources/Maps/Map_002.txt");
        auto field0 = std::make_shared<FieldRoom>(Room::Cfg{ 1, "Field", 50, 200 }, field0Map);
        field0Map->DefinePortal(PortalLink{
            2001, /* srcPortalId */
            0, /* dstMapId */
            1001, /*dstPortalId */
        });
        field0Map->MapTileToPortal(3, 3, 2001);
        field0->SetOwner(gQueues[OwnerIndexForRoom(field0->RoomId(), roomsPerQueue)].get());
        field0->StartTicking(); // 매 틱마다 호출하는 함수
        RoomManager::Instance().Add(field0);
    }
}

void StopShardedQueues()
{
    for (auto& q : gQueues) {
        q->Push(nullptr); // Pop이 nullptr 반환 → 워커 루프 종료
    }
    GThreadManager->Join();
    gQueues.clear();
}
