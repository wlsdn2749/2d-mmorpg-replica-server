#pragma once

extern class ThreadManager*				GThreadManager;
extern class Memory*					GMemory;
extern class SendBufferManager*			GSendBufferManager;
extern class GlobalQueue*				GGlobalQueue;
extern class JobTimer*					GJobTimer;

extern class DeadLockProfiler*			GDeadLockProfiler;
extern class DBConnectionPool*			GDBConnectionPool;
extern class ConsoleLogger*				GConsoleLogger;