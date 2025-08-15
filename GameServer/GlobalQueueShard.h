#pragma once

class JobQueue;

class GlobalQueueShard {
public:
	void Push(const shared_ptr<JobQueue>& jq)
	{
		std::cout << "[Shard] Push 실행 - qsize=" << _q.size() + 1 << " jq=" << jq.get() << std::endl;
		LockGuard lg(_mtx);
		_q.push(jq);
		_cv.notify_one();
	}

	JobQueueRef Pop(uint32 timeoutMs = 10)
	{
		UniqueLock ul(_mtx);
		if (!_cv.wait_for(ul, std::chrono::milliseconds(timeoutMs), [&] { return !_q.empty() || _stopping; }))
			return nullptr;
		if (_stopping) return nullptr;
		auto jq = _q.front();
		_q.pop();
		return jq;
	}

	void Stop()
	{
		{
			LockGuard lg(_mtx);
			_stopping = true;
			_cv.notify_all();
		}
	}

private:
	Mutex _mtx;
	CondVar _cv;
	Queue<JobQueueRef> _q;
	bool _stopping = false;

};