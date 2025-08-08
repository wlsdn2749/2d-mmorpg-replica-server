#include "pch.h"
#include <iostream>
#include "GameSession.h"
#include "GameSessionContainer.h"
#include "GameSessionAccessor.h"

#include "Service.h"
#include "ThreadManager.h"

#include "AppConfig.h"
#include "JwtAuth.h"
#include "ClientPacketHandler.h"

enum
{
	WORKER_TICK = 64
};

void DoWorkerJob(ServerServiceRef& service)
{
	static uint64 lastMemUpdateTick = 0; // 메모리 측정 주기 관리
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직까지 (패킷 핸들러)
		service->GetIocpCore()->Dispatch(10);

		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐 (게임에서 처리되는 일감들을 처리함)
		ThreadManager::DoGlobalQueueWork();
	}
}

int main()
{
	// TODO: DB Initialization

	// Room?

	ClientPacketHandler::init(); // 핸들러와 Wrapper 매핑 필수

	AppSettings s = LoadAppSettings();
	JwtAuth::Init(s.jwtSecret);

	GameSessionContainerRef container = MakeShared<GameSessionContainer>();
	GameSessionAccessor accessor(container); // 레퍼런스 주입

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"0.0.0.0", 6100),
		MakeShared<IocpCore>(),
		[container] {return MakeShared<GameSession>(container);},
		100);

	ASSERT_CRASH(service->Start());
	for (int32 i = 0; i < 5; i++) // worker count
	{
		GThreadManager->Launch([&service]()
			{
				while (true)
				{
					DoWorkerJob(service);

				}
			});
	}

	GThreadManager->Join();
}
