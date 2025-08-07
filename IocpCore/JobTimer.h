#pragma once

struct JobData
{
	JobData(weak_ptr<JobQueue> owner, JobRef job) : owner(owner), job(job)
	{
		
	}
	weak_ptr<JobQueue>	owner;
	JobRef				job;
};

struct TimerItem
{
	// Operator에서 사용하는 Less가 내림차순으로 정렬되므로, 여기서는 반대로 해주어야함
	bool operator<(const TimerItem& other) const
	{
		return executeTick > other.executeTick;
	}

	uint64 executeTick = 0;
	JobData* jobData = nullptr;
};

/*--------------
	JobTimer
--------------*/

class JobTimer
{
public:
	void Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job);
	void Distribute(uint64 now);
	void Clear();

private:
	USE_LOCK;
	PriorityQueue<TimerItem>	_items;
	Atomic<bool>				_distributing = false;

};

