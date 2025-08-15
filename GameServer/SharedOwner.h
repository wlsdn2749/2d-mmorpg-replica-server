#pragma once
#include "IJobQueueOwner.h"
#include "GlobalQueueShard.h"

class ShardOwner : public IJobQueueOwner {
public:
	explicit ShardOwner(shared_ptr<GlobalQueueShard> shard)
		: _shard(std::move(shard)) { }
	void Push(const JobQueueRef& jq) override
	{
		_shard->Push(jq);
	}

private:
	shared_ptr<GlobalQueueShard> _shard;
};