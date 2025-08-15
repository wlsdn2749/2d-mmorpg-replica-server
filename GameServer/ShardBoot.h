#pragma once
#include <cstdint>

void StartShardedQueues(int totalRooms, int roomsPerQueue, uint32 timeBudgetMs);
void CreateRooms(); // 룸 생성/배정/틱 시작
void StopShardedQueues(); // 종료용