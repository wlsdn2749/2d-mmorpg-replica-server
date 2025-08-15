#pragma once
#include <memory>

class JobQueue;

struct IJobQueueOwner {
	virtual ~IJobQueueOwner() = default;
	virtual void Push(const std::shared_ptr<JobQueue>& jq) = 0;
};