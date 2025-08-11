#include "pch.h"
#include "DBWorker.h"
#include "DBConnection.h"
#include <DBConnGuard.h>

#include "ThreadManager.h"

namespace {
	Atomic<bool> g_running(false);
}

static constexpr size_t MAX_BATCH = 256;

static void DbThreadMain()
{
	DBConnGuard threadConn;
	auto& conn = threadConn.ref();
	
	g_running.store(true, std::memory_order_release);

	while (g_running.load(std::memory_order_acquire))
	{
		Vector<DbJob> batch;
		size_t drained = GDbQueue.pop_all(batch);

		if (drained == 0) {
			DbJob j;
			if (!GDbQueue.wait_pop(j)) break;
			batch.push_back(std::move(j));
		}
		if(batch.size() > MAX_BATCH)
			batch.resize(MAX_BATCH);

		for (auto& job : batch) {
			try { job(conn); }
			catch (const std::exception& e) {
				GConsoleLogger->WriteStdOut(Color::YELLOW, L"[DB] Job exception : %s\n", StrToWstr(e.what()));
			} catch ( ... ){
				GConsoleLogger->WriteStdOut(Color::YELLOW, L"[DB] Job unknown exception\n");
			}
		}

		batch.clear();
	}
}

namespace DB {
	void Start() {
		GThreadManager->Launch([]()
		{
			DbThreadMain();
		});
	}

	void Stop() {
		g_running.store(false, std::memory_order_release);
		GDbQueue.stop();
	}
}