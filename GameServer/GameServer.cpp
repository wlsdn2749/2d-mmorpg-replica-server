#include "pch.h"
#include <iostream>

/*------------------------
	 네트워크 & 세션
------------------------*/
#include "GameSession.h"
#include "GameSessionContainer.h"
#include "GameSessionAccessor.h"

/*------------------------
	 서비스 & 스레드
------------------------*/
#include "Service.h"
#include "ThreadManager.h"

/*------------------------
	 설정 & 인증
------------------------*/
#include "AppConfig.h"
#include "JwtAuth.h"
#include "ClientPacketHandler.h"

/*------------------------
	 데이터 베이스
------------------------*/
#include "DBConnectionPool.h"
#include "DBSynchronizer.h"
#include "GlobalQueue.h"
#include "DBWorker.h"
enum
{
	WORKER_TICK = 64
};

void DoWorkerJob(ServerServiceRef& service)
{
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
	// Initialize AppSettings
	AppSettings s = LoadAppSettings(); 

	// Initialize & Sync DB
	{
		ASSERT_CRASH(GDBConnectionPool->Connect(10, StrToWstr(s.dbLocalPath).data()));
		DBConnection* dbConn = GDBConnectionPool->Pop();
		DBSynchronizer dbSync(*dbConn);
		dbSync.Synchronize(StrToWstr(s.dbXmlPath).data());
		GDBConnectionPool->Push(dbConn);

		DB::Start();
	}

	// Initalize Handler & Valiator
	ClientPacketHandler::init(); // 핸들러와 Wrapper 매핑 필수
	JwtAuth::Init(s.jwtSecret);

	// TODO Room


	GameSessionContainerRef container = MakeShared<GameSessionContainer>();
	GameSessionAccessor accessor(container); // 레퍼런스 주입

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"0.0.0.0", 6201),
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
