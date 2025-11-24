#pragma once

template<typename T>
class Singleton
{
public:
	static T& Instance()
	{
		static T _instance;
		return _instance;
	}

	Singleton(const Singleton& other) = delete;
	Singleton& operator=(const Singleton& other) = delete;
protected:
	Singleton() = default;
	~Singleton() = default;

};