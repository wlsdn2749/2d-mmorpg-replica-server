#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job)
{
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

	WRITE_LOCK;
	_items.push(TimerItem { executeTick, jobData });
}

void JobTimer::Distribute(uint64 now)
{
	// 한 번에 1 스레드만 통과

	if (_distributing.exchange(true) == true)
		return; // 이전값이 true이면 였으면 그냥 리턴, 아니면 통과

	Vector<TimerItem> items;
	{
		WRITE_LOCK;

		while (_items.empty() == false)
		{
			const TimerItem& timerItem = _items.top();
			if (now < timerItem.executeTick)
				break;

			items.push_back(timerItem);
			_items.pop();
			
		}
	}

	for (TimerItem& item : items)
	{
		if (JobQueueRef owner = item.jobData->owner.lock()) // item의 소유자를 체크해서 JobQueueRef에 넣고, 그걸 nullptr체크까지
		{
			owner->Push(item.jobData->job);
		}
		ObjectPool<JobData>::Push(item.jobData);
	}


	// 끝났으면 풀어준다.
	_distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;
	while (_items.empty() == false)
	{
		const TimerItem& timerItem = _items.top();
		ObjectPool<JobData>::Push(timerItem.jobData);
		_items.pop();
	}
}
