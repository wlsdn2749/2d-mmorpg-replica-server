# 2D MMORPG Server

## 프로젝트 개요

고성능 멀티스레드 MMORPG 서버 시스템 구현 프로젝트입니다. IOCP(I/O Completion Port) 기반의 비동기 네트워킹과 JobQueue를 활용한 Lock-free 아키텍처를 통해 높은 동시성과 확장성을 확보했습니다.

이 프로젝트는 학습 및 포트폴리오를 목적으로 개발되었으며, 실제 게임 서버에서 요구되는 핵심 기능들을 구현하고 있습니다.

## Quick Start
1. Visual Studio 2022 필요
Installer에서 Python 개발, .NET 데스크톱 개발, C++ 사용한 데스크톱 개발, 데이터 스토리지 및 처리 설치

2. Shared 디렉토리에 있는 `authserver.appsettings.json`, `gameserver.appsettings.json`, `sheetloader.example_service_key.json`를
로컬 설정에 맞게 변경 후, 이름을 `appsettings.json`과 `service-account-key.json`으로 변경 후 각 폴더의 루트에 넣습니다. 

3. DB(ms-sql) 기준 AuthDb와 GameDb를 만들어줍니다. 테이블과 프로시져는 ORM으로 빌드 시 생성됩니다.
4. .sin을 열어 빌드시켜줍니다. Client는 DummyClient나 TestClientUnity(GUI)를 사용하시면 됩니다.




## 주요 기술 스택

- **IOCP 기반 C++ 게임 서버**: Windows IOCP를 활용한 고성능 멀티스레드 비동기 I/O 처리
- **gRPC + JWT 인증 시스템**: .NET 9.0 기반 인증 서버, JWT 토큰 기반 보안
- **Protocol Buffers**: 효율적인 바이너리 직렬화 및 클라이언트-서버 통신
- **자동화된 코드 생성**:
  - PacketGenerator: Protocol Buffers 정의로부터 C++ Handler 및 C# Manager 자동 생성
  - ProcedureGenerator: XML 스키마로부터 ORM Stored Proced ure 래퍼 자동 생성
- **Google Sheets 연동**: 게임 데이터(몬스터, 아이템 등)를 Google Sheets에서 JSON으로 자동 배포

## 구현된 게임 시스템

- **인벤토리 시스템**: 40슬롯, 아이템 스택, 실시간 DB 동기화
- **장비 시스템**: 장착/해제 기능, 스탯 적용, 레벨 제한
- **파티 시스템**: 파티 초대/수락/거절, 공개 파티, 강퇴 기능
- **레벨/전투 시스템**: 경험치, 레벨업, 플레이어-몬스터 전투
- **채팅 시스템**: 룸별 채팅, 전체 채팅
- **멀티 룸 시스템**: 포털을 통한 맵 이동, 지역별 룸 매핑
- **몬스터 AI FSM**: Patrol, Chase, Combat 상태 기반 몬스터 AI
- **NPC 시스템**: Component 기반 NPC (Dialog, Shop, Quest)
- **드롭 시스템**: 몬스터 처치 시 아이템 드롭

## 스레드 아키텍처 및 동시성 제어

### 3단계 스레드 분리

본 프로젝트는 작업 유형에 따라 스레드를 3단계로 분리하여 효율적인 멀티스레드 처리를 구현했습니다.

- **IOCP Worker (5개)**: 네트워크 I/O 전담
  - `IocpCore::Dispatch()`를 통해 소켓 읽기/쓰기 완료 이벤트 처리
  - 패킷을 수신하면 해당 Room의 JobQueue에 작업을 위임하고 즉시 다음 I/O 처리

- **Shard Worker (2개)**: Room별 게임 로직 처리
  - 각 Shard는 2개의 Room을 담당 (총 3개 Room → 2개 Shard)
  - `GlobalQueue`에서 JobQueue를 꺼내 실행하는 방식
  - Room 내부 상태(플레이어, 몬스터 등)는 해당 Shard Worker에서만 접근

- **DB Worker (1개)**: 비동기 데이터베이스 작업
  - 캐릭터 저장, 인벤토리 업데이트, 장비 변경 등 모든 DB 작업 처리
  - `std::packaged_task`와 `std::future`를 활용한 비동기 작업 처리

### Lock-Free 동작 구조

이 서버는 대부분의 게임 로직에서 **명시적인 Lock 없이 동작**합니다.

```
[클라이언트 패킷 수신]
        ↓
[IOCP Worker] ← Packet 수신
        ↓
  Room->DoAsync(Lambda)  ← JobQueue에 작업 추가
        ↓
[Shard Worker] ← JobQueue에서 작업 꺼내 실행
        ↓
[Room 내부 로직] ← _players, _monsters 등 Room 상태 접근
```

**핵심 원리:**
- 각 Room은 독립적인 `JobQueue`를 보유
- IOCP 스레드는 패킷 수신 시 `Room->DoAsync(lambda)`로 작업을 JobQueue에 추가하고 즉시 반환
- Room 내부 데이터(`_players`, `_monsters` 등)는 **해당 Room을 담당하는 Shard Worker에서만 접근**
- **단일 스레드 접근 보장으로 Lock이 불필요**하며, JobQueue를 통한 비동기 처리로 높은 동시성 확보

### 데이터베이스 계층

#### DbDispatcher: `std::packaged_task` & `std::future` 기반 비동기 처리

```cpp
auto future = DbDispatcher::EnqueueRet([=](DBConnection& conn) {
    // DB 작업 수행
    return CharacterRepository::GetCharacterStat_DB(conn, characterId);
});

// 게임 로직 계속 진행 가능
// ...

// 필요 시점에 결과 대기
auto result = future.get();
```

- DB 작업을 별도 스레드에서 처리하여 게임 로직 블로킹 방지
- `packaged_task`로 작업을 패키징하고, `future`로 결과 수신

#### Repository Pattern: 일관성 있는 DB 진입점

모든 DB 작업은 Repository 클래스를 통해 접근하여 코드 일관성과 유지보수성을 확보했습니다.

- **CharacterRepository**: 캐릭터 생성, 조회, 업데이트, 삭제
- **InventoryRepository**: 인벤토리 아이템 CRUD
- **EquipmentRepository**: 장비 인스턴스 및 장착 상태 관리
- **AccountRepository**: 계정 인증 및 사용자 정보
- **DropTableRepository**: 몬스터 드롭 테이블 관리

#### XML 기반 자동 ORM 생성

`GameDB.xml`에 정의된 테이블 스키마와 Stored Procedure를 기반으로 `GenProcedures.h`가 자동 생성됩니다. 이를 통해 타입 안전성과 개발 생산성을 확보했습니다.

## 아키텍처 특징

- **Room 기반 멀티스레드 처리**: 각 Room은 독립적인 JobQueue를 통해 게임 로직을 비동기 처리
- **Singleton Manager/Service 구조**: 중앙화된 관리 클래스로 시스템 간 통신 및 상태 관리
- **자동 저장 시스템**:
  - 3분 주기 자동 저장
  - 룸 전환 시 자동 저장
  - 연결 해제 시 자동 저장
- **Component 기반 NPC 시스템**: Dialog, Shop, Quest 등 기능을 Component로 분리하여 확장 가능한 구조

## 개발 효율성 도구

### PacketGenerator (Python)

Protocol Buffers 정의 파일(`.proto`)을 파싱하여 C++ 패킷 핸들러와 C# 패킷 매니저를 자동 생성합니다.

```bash
cd Protocol
GenPackets.bat
```

**생성 파일:**
- `ClientPacketHandler.h/cpp`: C++ 서버 패킷 핸들러
- `PacketManager.cs`: C# 클라이언트 패킷 매니저

### ProcedureGenerator (Python)

`GameDB.xml`을 파싱하여 ORM Stored Procedure 래퍼를 자동 생성합니다.

```bash
cd Common/Procedures
GenProcs.bat
```

**생성 파일:**
- `GenProcedures.h`: Stored Procedure 래퍼 클래스

### SheetLoader

Google Sheets에서 게임 데이터(몬스터 스탯, 아이템 정보 등)를 JSON으로 자동 다운로드하여 서버에 배포합니다.

## 폴더 및 파일 구조

### 디렉토리 구조

```
root/
├── AuthServer/              # gRPC 인증 서버 (.NET 9.0)
├── GameServer/              # TCP 게임 서버 (C++)
├── IocpCore/                # IOCP 네트워킹 코어 라이브러리
├── DummyClientCS/           # C# 테스트 클라이언트
├── TestClientUnity/         # Unity 클라이언트
├── Protocol/                # Protocol Buffers 정의 및 생성 스크립트
├── Common/Procedures/       # DB Procedure 생성 도구
├── Tools/                   # 코드 생성 유틸리티
└── third_party/             # 외부 의존성 (gRPC, protobuf 등)
```

### GameServer 파일 구조 (이름 패턴별 분류)

GameServer는 물리적 폴더 분리 없이 파일 이름 패턴으로 논리적 분류를 합니다.

#### Session Management
- `GameSession`: 클라이언트 연결 세션
- `GameSessionContainer`: 세션 컨테이너
- `GameSessionAccessor`: 스레드 안전 세션 접근

#### Player System
- `Player`: 플레이어 엔티티
- `PlayerCombatSystem`: 플레이어 전투 로직
- `PlayerPorts`: 플레이어 인터페이스

#### Monster System
- `MonsterService`: 몬스터 서비스
- `MonsterContainer`: 몬스터 컨테이너
- `MonsterCombatSystem`: 몬스터 전투 AI
- `MonsterMovementSystem`: 몬스터 이동 로직
- `MonsterSpawnerSystem`: 몬스터 스폰 시스템
- `MonsterTypes`, `MonsterPorts`: 몬스터 타입 정의
- `MonsterDataParser`: 몬스터 데이터 파싱

#### Room System
- `Room`: 룸 베이스 클래스
- `FieldRoom`: 전투 필드 룸
- `TownRoom`: 마을 룸
- `RoomManager`: 룸 관리

#### Database Layer
- `DBWorker`: DB 작업 워커 스레드
- `DBDisPatcher`: DB 작업 디스패처 (PackagedTask & Future)
- `AccountRepository`: 계정 레포지토리
- `CharacterRepository`: 캐릭터 레포지토리
- `InventoryRepository`: 인벤토리 레포지토리
- `EquipmentRepository`: 장비 레포지토리
- `DropTableRepository`: 드롭 테이블 레포지토리
- `GenProcedures`: 자동 생성 ORM SP 래퍼

#### Inventory & Equipment System
- `InventoryCore`, `InventorySystem`: 인벤토리 핵심 로직
- `EquipmentCore`, `EquipmentSystem`: 장비 핵심 로직
- `EquipmentManager`: 장비 관리자
- `EquipmentDataParser`: 장비 데이터 파싱

#### Item & Drop System
- `ItemManager`: 아이템 관리자
- `ItemDataParser`: 아이템 데이터 파싱
- `DropManager`: 드롭 관리자
- `DropDataParser`: 드롭 데이터 파싱

#### Party System
- `Party`: 파티 엔티티
- `PartyService`: 파티 서비스
- `PartyManager`: 파티 관리자

#### NPC System
- `Npc`: NPC 엔티티
- `NpcManager`: NPC 관리자
- `NpcDialogComponent`, `NpcDialogManager`: NPC 대화 시스템
- `NpcQuestComponent`: NPC 퀘스트 컴포넌트
- `NpcShopComponent`, `NpcShopDataManager`: NPC 상점 시스템

#### Map & Spawn System
- `MapData`: 맵 데이터 로딩 및 관리
- `SpawnPointDataParser`: 스폰 포인트 파싱

#### Network & Protocol
- `ClientPacketHandler`: 자동 생성 패킷 핸들러
- `ProtocolHelper`: 프로토콜 헬퍼 함수
- `Protocol.pb`: 자동 생성 Protocol Buffers 클래스

#### Configuration & Authentication
- `AppConfig`: 애플리케이션 설정
- `JwtAuth`: JWT 토큰 검증
- `Account`: 계정 엔티티

#### Thread Management & Shard System
- `ShardBoot`: Shard 초기화 및 Room 생성
- `GlobalQueueShard`: Shard별 GlobalQueue 관리
- `SharedOwner`: 공유 리소스 소유권 관리
- `IJobQueueOwner`: JobQueue 소유자 인터페이스

#### Core Utilities
- `EntityCore`: 엔티티 핵심 로직
- `GeometryCore`: 2D 좌표, 방향, 충돌 처리
- `TypeCore`: 핵심 타입 정의
- `TimeUtils`: 시간 유틸리티
- `RandomUtils`: 랜덤 유틸리티
- `JsonFileUtils`, `JsonDataParser`: JSON 파싱
- `StringConvert`: 문자열 변환

### IocpCore 파일 구조

IocpCore는 재사용 가능한 네트워킹 라이브러리로, 게임 서버뿐 아니라 다른 서버에도 활용 가능합니다.

#### Core IOCP System
- `IocpCore`: IOCP 메인 클래스
- `IocpEvent`: IOCP 이벤트 처리
- `Service`: 네트워크 서비스 베이스
- `Session`: 세션 베이스 클래스
- `Listener`: TCP 리스너

#### Memory Management
- `Memory`: 메모리 관리
- `MemoryPool`: 메모리 풀링
- `Allocator`: 커스텀 할당자
- `ObjectPool`: 객체 풀링

#### Threading & Synchronization
- `ThreadManager`: 스레드 풀 관리
- `Lock`: SpinLock 등 락 구현
- `LockQueue`: 스레드 안전 큐
- `DeadLockProfiler`: 데드락 프로파일러

#### Job System
- `Job`: 작업 베이스 클래스
- `JobQueue`: 작업 큐
- `JobTimer`: 타이머 기반 작업 스케줄링
- `GlobalQueue`: 전역 작업 큐

#### Network Buffers
- `SendBuffer`: 송신 버퍼
- `RecvBuffer`: 수신 버퍼
- `BufferReader`: 바이너리 읽기
- `BufferWriter`: 바이너리 쓰기

#### Database Integration
- `DBConnection`: MySQL 연결 래퍼
- `DBConnectionPool`: 연결 풀링
- `DBBind`: SQL 파라미터 바인딩
- `DBModel`: DB 모델 베이스
- `DBSynchronizer`: DB 동기화
- `DBConnGuard`: RAII 연결 가드

#### Network Utilities
- `NetAddress`: 네트워크 주소 처리
- `SocketUtils`: 소켓 유틸리티

#### Core Infrastructure
- `CoreGlobal`: 전역 변수
- `CorePch`: 프리컴파일 헤더
- `CoreTLS`: 스레드 로컬 스토리지
- `CoreMacro`: 공통 매크로
- `Types`: 타입 정의
- `TypeCast`: 타입 캐스팅
- `Container`: STL 컨테이너 정의

#### Utilities
- `FileUtils`: 파일 I/O
- `ConsoleLogger`: 콘솔 로깅
- `XMLParser`: XML 파싱 (RapidXML)

## 빌드

설정 파일을 Github에 업로드 했습니다.

크게 2가지 환경 종속적인 부분이 있습니다.

### 1. DB

DB의 경우 로컬 SQL Server의 경로, driver 등 설치되어 있는 경로를 정확히 입력해야 하며, 이를 잘못 입력할 시 빌드가 되지 않을 수 있습니다.

또한 빌드 전에 SQL에 직접 접근해 `GameDb`와 `AuthDb` 데이터베이스를 생성해야 하고, 빌드 시 GameServer의 경우 `GameDB.xml`을 통해 자동으로 ORM이 생성되지만, AuthServer의 경우 없습니다. 

아래의 T-SQL을 사용하여 직접 Table을 만들어주어야 합니다

```SQL
CREATE TABLE [dbo].[Users]
(
	[Id] INT NOT NULL IDENTITY(1,1) PRIMARY KEY,
	[Email] NVARCHAR(100) NOT NULL UNIQUE,
	[PasswordHash] NVARCHAR(200) NOT NULL,
	[CreatedAt] DATETIME2(0) NOT NULL
				CONSTRAINT DF_Users_CreatedAt
				DEFAULT (SYSUTCDATETIME())
)
```

### 빌드 순서

```bash

# Visual Studio 솔루션 빌드
msbuild 2d-mmorpg-replica.sln /p:Configuration=Debug /p:Platform=x64
```

혹은 **.sin 파일을 visual studio 2022로 열어 직접 빌드**

빌드를 하게 되면 빌드 전 이벤트로 ORM & Proto 파일 등 자동생성되니 빌드는 필수 입니다.