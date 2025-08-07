#pragma once

#include <thread>
#include <functional>

/*------------------
	ThreadManager
-------------------*/
class ThreadManager
{

public:
	ThreadManager();
	~ThreadManager();

	void	Launch(std::function<void(void)> callback);
	void	Join();

	static void initTLS();
	static void DestroyTLS();
	static void DistributeReservedJobs();


	static void DoGlobalQueueWork();
private:
	Mutex						_lock;
	std::vector<std::thread>	_threads;
};

