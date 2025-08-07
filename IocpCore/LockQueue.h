#pragma once

template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		WRITE_LOCK;
		_item.push(item);
	}

	T Pop()
	{
		WRITE_LOCK;
		if (_item.empty())
		{
			return T();
		}

		T ret = _item.front();
		_item.pop();
		return ret;
	}
	void PopAll(OUT Vector<T>& items)
	{
		WRITE_LOCK;
		while (T item = Pop())
			items.push_back(item);
	}
	void Clear()
	{
		WRITE_LOCK;
		_item = Queue<T>();
	}
private:
	USE_LOCK;
	Queue<T> _item;
};