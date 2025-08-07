#pragma once

// 여기에 미리 컴파일하려는 헤더 추가
#define WIN32_LEAN_AND_MEAN

#ifdef _DEBUG
#pragma comment(lib, "IocpCore\\Debug\\IocpCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "IocpCore\\Release\\IocpCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif

#include "CorePch.h"

#include "Enum.pb.h"
#include "Struct.pb.h"

using GameSessionRef	= shared_ptr<class GameSession>;
using PlayerRef			= shared_ptr<class Player>;

using GameSessionContainerRef = shared_ptr<class GameSessionContainer>;
using GameSessionAccessorRef = shared_ptr<class GameSessionAccessor>;
