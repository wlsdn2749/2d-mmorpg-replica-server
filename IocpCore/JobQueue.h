#pragma once
#include "Job.h"
#include "JobTimer.h"
#include "LockQueue.h"
#include "../GameServer/IJobQueueOwner.h"
#include "../GameServer/GlobalQueueShard.h"


/*------------------------
		JobQueue
 -----------------------*/

class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	void DoAsync(CallbackType&& callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
	}

	//template<typename T, typename Ret, typename... Args>
	//void DoAsync(Ret(T::*memFunc)(Args...), Args... args)
	//{
	//	shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
	//	Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	//}	

	template<typename F>
	void DoAsync(F&& f)
	{
		Push(ObjectPool<Job>::MakeShared(std::forward<F>(f)));
	}


	// 클래스의 멤버함수 호출 비 const 버전
	// Ret: 리턴 타입
	// T: 클래스
	// T::*memFunc : 멤버함수 포인터
	// FArgs: 멤버 함수 시그니처의 매개변수 타입들
	// CallArgs: 실제 호출시 넘긴 인자들의 타입 template + &&로 받기때문에 그대로 전달되는 보편참조
	// 결과적으로 (*owner).*memFunc(forwarded args...) 로 실행
	template<typename T, typename Ret, typename... FArgs, typename... CallArgs>
	void DoAsync(Ret(T::* memFunc)(FArgs...), CallArgs&&... callArgs)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());

		auto task = [owner = std::move(owner), memFunc,
			tup = std::make_tuple(std::forward<CallArgs>(callArgs)...)]() mutable
			{
				std::apply([&](auto&&... args) {
					std::invoke(memFunc, owner.get(), std::forward<decltype(args)>(args)...);
					}, tup);
			};

		DoAsync(std::move(task)); // 위 콜러블 DoAsync로 위임
	}

	// ── (2) const 멤버 함수 포인터 ────────────────────────────────
	template<typename T, typename Ret, typename... FArgs, typename... CallArgs>
	void DoAsync(Ret(T::* memFunc)(FArgs...) const, CallArgs&&... callArgs)
	{
		std::shared_ptr<const T> owner = std::static_pointer_cast<const T>(shared_from_this());

		auto task = [owner = std::move(owner), memFunc,
			tup = std::make_tuple(std::forward<CallArgs>(callArgs)...)]() mutable
			{
				std::apply([&](auto&&... args) {
					std::invoke(memFunc, owner.get(), std::forward<decltype(args)>(args)...);
					}, tup);
			};

		DoAsync(std::move(task));
	}
	void DoTimer(uint64 tickAfter, CallbackType&& callback)
	{
		JobRef job = ObjectPool<Job>::MakeShared(std::move(callback));
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	template<typename T, typename Ret, typename... Args>
	void DoTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	void CledrJobs() {_jobs.Clear(); }

public:
	explicit JobQueue(GlobalQueue* owner = nullptr) : _owner(owner) {}

public:
	void Execute();
	void Push(JobRef job, bool pushOnly = false);
	void SetOwner(GlobalQueue* owner) { _owner = owner; }

protected:
	LockQueue<JobRef>	_jobs;
	Atomic<int32>		_jobCount = 0;
private:
	GlobalQueue* _owner = nullptr;
};

