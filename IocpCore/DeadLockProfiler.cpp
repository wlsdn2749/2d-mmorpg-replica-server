#include "pch.h"
#include "DeadLockProfiler.h"

/*------------------------
	DeadLockProfiler
-------------------------*/
void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	// 아이디를 찾거나 발급
	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end())
	{
		lockId = static_cast<int32>(_nameToId.size());
		_idToName[lockId] = name;
		_nameToId[name] = lockId;
	}
	else
	{
		lockId = findIt->second;
	}

	// 잡고 있는 락이 있었다면
	if (LLockStack.empty() == false)
	{
		// 기존에 발견되지 않은 케이스라면 데드락 여부 다시 확인
		const int32 prevId = LLockStack.top();
		if (lockId != prevId)
		{
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end())
			{
				history.insert(lockId);
				CheckCycle();
			}
		}
	}
	LLockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{

	LockGuard guard(_lock);

	if (LLockStack.empty())
		CRASH("MULTIPLE UNLOCK");

	int32 lockId = _nameToId[name];
	if (LLockStack.top() != lockId)
		CRASH("INVAILD LOCK");

	LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());

	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; ++lockId)
	{
		Dfs(lockId);
	}

	// 연산이 끝났으면 정리
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1) // 
	{
		return;
	}

	_discoveredOrder[here] = _discoveredCount++; // here에 0 (부터 시작)을 넣고 증가시켜줌

	// 모든 인접한 정점을 순회

	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end()) // 해당 락을 잡은 상태에서, 다른 락을 잡은적이 없다
	{
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		// 아직 방문한 적 없으면 방문한다.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// here가 there보다 먼저 발견되었다면, there는 here의 후손이다. (순방향 간선)
		if (_discoveredOrder[here] < _discoveredOrder[there])
		{
			continue;
		}

		// 순방향이 아니고, Dfs(there)가 아직 종료되지 않았다면, there는 here의 선조이다. (역방향 간선)
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);

			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}
			CRASH("DEADLOCK_DETECTED")
		}
	}
	_finished[here] = true;

}
