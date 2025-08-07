#pragma once
#include <mutex>
#include <atomic>

using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;


template<typename T>
using Atomic = std::atomic<T>;
using Mutex = std::mutex;
using CondVar = std::condition_variable;
using UniqueLock = std::unique_lock<std::mutex>;
using LockGuard = std::lock_guard<std::mutex>;

// #define USING_SHARED_PTR(name) using name##Ref = std::shared_ptr<class name>;
#define USING_SHARED_PTR(name)	using name##Ref = std::shared_ptr<class name>;

USING_SHARED_PTR(IocpCore);
USING_SHARED_PTR(IocpObject);
USING_SHARED_PTR(Session);
USING_SHARED_PTR(PacketSession);
USING_SHARED_PTR(Listener);
USING_SHARED_PTR(ServerService);
USING_SHARED_PTR(ClientService);
USING_SHARED_PTR(SendBuffer);
USING_SHARED_PTR(SendBufferChunk);
USING_SHARED_PTR(Job);
USING_SHARED_PTR(JobQueue);

// sharedPtr
//using IocpObjectRef			= std::shared_ptr<class IocpObject>;
//using IocpCoreRef			= std::shared_ptr<class IocpCore>;
//using SessionRef			= std::shared_ptr<class Session>;
//using ListenerRef			= std::shared_ptr<class Listener>;
//using ServerServiceRef		= std::shared_ptr<class ServerService>;
//using ClientServiceRef		= std::shared_ptr<class ClientService>;
//using SendBufferRef			= std::shared_ptr<class SendBuffer>;
//using SendBufferChunkRef	= std::shared_ptr<class SendBufferChunk>;
//using PacketSessionRef		= std::shared_ptr<class PacketSession>;
//using JobRef				= std::shared_ptr<class Job>;
//using JobQueueRef			= std::shared_ptr<class JobQueue>;

#define size16(val) static_cast<int16>(sizeof(val))
#define size32(val) static_cast<int32>(sizeof(val))
#define len16(arr)	static_cast<int16>(sizeof(arr)/sizeof(arr[0]))
#define len32(arr)	static_cast<int32>(sizeof(arr)/sizeof(arr[0]))
#define _STOMP