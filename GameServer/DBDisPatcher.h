#pragma once
#include <future>
#include <utility>


class DBConnection;

using DbJob = std::function<void(DBConnection&)>;

// 스레드 안전 큐
class DbQueue {
public:
	void push(DbJob job)
	{
		{
			LockGuard lg(_lock);
			_q.push(std::move(job));
		}
		_cv.notify_one(); // 넣었으면 유휴 스레드 깨우도록 cv 변수 변경
	}

	bool wait_pop(DbJob& out)
	{
		UniqueLock ul(_lock);
		_cv.wait(ul, [&] {return _stop || !_q.empty(); });
		// 다른 스레드가 _cv.notify_one() or notify_all()을 통해
		// 깨웠을때 _stop이거나 큐가 비어있지 않으면 깨어나 동작함
		if (_stop && _q.empty()) return false;
		out = std::move(_q.front());
		_q.pop();
		return true;
	}

	// 한번에 모두 꺼내기 (락 최소화)
	size_t pop_all(Vector<DbJob>& out)
	{
		Queue<DbJob> local;
		{
			LockGuard lg(_lock);
			swap(local, _q);
		}

		size_t n = 0;
		while (!local.empty()) {
			out.push_back(std::move(local.front()));
			local.pop();
			++n;
		}
		return n;
	}

	void stop() {
		{
			LockGuard lg(_lock);
			_stop = true;
		}
		_cv.notify_all();
	}

private:
	Mutex _lock;
	CondVar _cv;
	Queue<DbJob> _q;
	bool _stop{ false };
};

extern DbQueue GDbQueue;
struct DbDispatcher {
	static std::future<void> Enqueue(std::function<void(DBConnection&)> fn)
	{
		auto taskPtr = MakeShared<std::packaged_task<void(DBConnection&)>>(std::move(fn));
		auto fut = taskPtr->get_future();
		GDbQueue.push([taskPtr](DBConnection& c) { (*taskPtr)(c); });
		return fut;
	}

	template<typename Fn>
	static auto EnqueueRet(Fn fn)
		-> std::future<decltype(fn(std::declval<DBConnection&>()))> // template의 반환타입을 future의 반환타팁으로 매핑
	{
		using R = decltype(fn(std::declval<DBConnection&>()));
		auto taskPtr = MakeShared<std::packaged_task<R(DBConnection&)>>(std::move(fn));
		auto fut = taskPtr->get_future();
		GDbQueue.push([taskPtr](DBConnection &c) mutable {
			(*taskPtr)(c);
		});
		return fut;
	}
};
