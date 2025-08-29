# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Key Rules
1. 모든 대답은 한국어로 한다.
2. 한국어로 변환하기 힘든 단어인 경우 판단하에 영어로 표기한다.
3. 특별히 언급하지 않는한 코드베이스는 IMPORTANT "GameServer/ DummyClientCS/ IocpCore/ Protocol/" 이 4개의 폴더만을 참조한다.
4. 

## The Golden Rule

When unsure about implementation details, ALWAYS ask the developer.
## Build Commands

**Visual Studio Solution Build:**
```bash
# Build the entire solution
msbuild 2d-mmorpg-replica.sln /p:Configuration=Debug /p:Platform=x64
msbuild 2d-mmorpg-replica.sln /p:Configuration=Release /p:Platform=x64

# Build specific projects
msbuild GameServer/GameServer.vcxproj /p:Configuration=Debug /p:Platform=x64
msbuild AuthServer/AuthServer.csproj /p:Configuration=Debug
msbuild DummyClientCS/DummyClientCS.csproj /p:Configuration=Debug
```

**Code Generation:**
```bash
# Generate database procedures (must run before building GameServer)
cd Common/Procedures
GenProcs.bat

# Generate protocol handlers and protobuf files
cd Protocol
GenPackets.bat
```

## Architecture Overview

This is a 2D MMORPG server implementation with the following key components:

### Core Components

- **GameServer (C++)**: TCP-based game server using IOCP for high-performance networking
- **AuthServer (C# .NET 9.0)**: gRPC-based authentication server with JWT tokens
- **IocpCore**: Core IOCP networking library - rarely needs modification
- **DummyClientCS (C#)**: Test client for gRPC + TCP communication testing

### Recent Updates
- **Player Combat System**: 플레이어 공격 시스템 구현 (C_PlayerAttackRequest 프로토콜)
- **Database Auto-Save**: 연결 해제/룸 전환/주기적 저장(3분) 시스템
- **Character Stats**: exp 컬럼 추가, lastRoom 네이밍 변경
- **JobQueue Enhancement**: DoTimer 인터페이스 개선 (람다/멤버함수 지원)

### Protocol & Code Generation

The system uses **Protocol Buffers** for client-server communication with automated code generation:

- `Protocol/Protocol.proto`: Main game protocol definitions
- `Protocol/auth.proto`: Authentication protocol definitions
- **PacketGenerator**: Python tool that auto-generates C++ packet handlers and C# packet managers
- **ProcedureGenerator**: Python tool that auto-generates SQL stored procedure wrappers from `GameDB.xml`

### Database Layer

- **GameDB.xml**: Database schema and stored procedures definition in XML format
- Automated ORM generation for GameServer (AuthServer uses manual Dapper)
- Two databases: `GameDb` and `AuthDb` (must be created manually in MySQL)

### Key Architecture Patterns

1. **Entity Component System**: Players and monsters use entity-based architecture
2. **Room System**: Game world divided into rooms with separate processing threads
3. **Job Queue System**: Asynchronous task processing using `JobQueue` with `DoAsync` interface
4. **Combat System**: Separate combat systems for players (`PlayerCombatSystem`) and monsters (`MonsterCombatSystem`)

### Directory Structure
```
root/
├── AuthServer/              # gRPC authentication server (.NET 9.0)
├── GameServer/              # Main TCP game server (C++)
├── IocpCore/                # IOCP networking core library
├── DummyClientCS/           # C# test client
├── TestClientUnity/         # Unity client project
├── Protocol/                # Protocol definitions and generation scripts
├── Common/Procedures/       # Database procedure generation tools
├── Tools/                   # Code generation utilities
└── third_party/             # External dependencies (gRPC, protobuf, etc.)
```

## Development Workflow

1. **Schema Changes**: Modify `GameServer/GameDB.xml` → Run `Common/Procedures/GenProcs.bat`
2. **Protocol Changes**: Modify `Protocol/Protocol.proto` → Run `Protocol/GenPackets.bat`
3. **Building**: Ensure code generation is run before building projects
4. **Database Setup**: Manually create `GameDb` and `AuthDb` in MySQL before first run

## Environment Dependencies

1. **Database**: Local MySQL with correct driver paths configured
2. **IP Configuration**: Server opens on `0.0.0.0:Port` but may have IP dependencies
3. **Build Requirements**: Visual Studio with C++ and .NET toolchain, Python for code generation

## Important Files

- `GameServer/GenProcedures.h`: Auto-generated database procedures (do not edit manually)
- `GameServer/Protocol.pb.h/.cc`: Auto-generated protobuf files
- `GameServer/ClientPacketHandler.h`: Auto-generated packet handlers
- Various `*PacketManager.cs` files: Auto-generated C# packet managers

## Notes

- The `IocpCore` library is stable and rarely needs modification
- All protocol and database code is auto-generated - edit the source files (`.proto`, `.xml`) instead
- Recent commits show active development on player stats, combat systems, and monster AI
- The project uses both C++ (server performance) and C# (tooling/clients) components