#pragma once

#include "Types.h"
#include "CoreMacro.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "Container.h"

#include <windows.h>
#include <chrono>

using namespace std;

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Lock.h"
#include "ObjectPool.h"
#include "TypeCast.h"
#include "Memory.h"

#include "SendBuffer.h"
#include "Session.h"
#include "LockQueue.h"
#include "ConsoleLogger.h"
class Uncopyable
{
protected:
	Uncopyable() = default;
	virtual ~Uncopyable() = default;

private:
	Uncopyable(const Uncopyable& u) = delete; // 복사 생성자 삭제
	Uncopyable& operator=(const Uncopyable& u) = delete; //복사 대입 연산자 삭제
};