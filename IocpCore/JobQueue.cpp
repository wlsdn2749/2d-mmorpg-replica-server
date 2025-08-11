#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"


/*------------------------
		JobQueue
 -----------------------*/

void JobQueue::Push(JobRef job, bool pushOnly)
{

	const int32 prevCount = _jobCount.fetch_add(1); // 카운트 증가
	_jobs.Push(job); // WRITE_LOCK // 잡 실행

	// 첫번째 job을 넣은 쓰레드
	if (prevCount == 0)
	{
		// 이미 실행중인 JobQueue가 없으면 실행
		if (LCurrentJobQueue == nullptr && pushOnly == false)
		{
			Execute();
		}
		else
		{
			// 여유 있는 다른 스레드가 실행하도록 GlobalQueue에 넘김
			// 실행목적아니고 그냥 쌓는게 목적임 (JobTimer)
			if(pushOnly == false && _owner)
				_owner->Push(shared_from_this());

			//GGlobalQueue->Push(shared_from_this());
		}
	}
}


// 1) 일감이 너무 몰리면?
// 2) DoAsync 타고 가서 절때 끝나지 않는 상황? -> 일감이 한 스레드한테 물림


void JobQueue::Execute()
{

	LCurrentJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());

		//cout << "JobCount는? " << jobCount << endl;

		for (int32 i = 0; i < jobCount; i++)
		{
			jobs[i]->Execute(); // 잡 실행

		}
		

		// 남은 일감 0개라면 종료
		if (_jobCount.fetch_sub(jobCount) == jobCount) // 카운트 증감
		{
			LCurrentJobQueue = nullptr;
			return;	
		}

		const uint64 now = ::GetTickCount64();
		if (now >= LEndTickCount)
		{
			LCurrentJobQueue = nullptr;
			// 여유 있는 다른 스데르가 실행하도록 GlobalQueue에 넘김
			if(_owner) _owner->Push(shared_from_this());
			break;
		}
	}
}
