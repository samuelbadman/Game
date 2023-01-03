#pragma once

template<typename... Args>
class callback
{
private:
	std::vector<void(*)(Args... args)> registeredCallbacks;

public:
	void add(void(*func)(Args... args))
	{
		registeredCallbacks.push_back(func);
	}

	void broadcast(Args... args)
	{
		for (void(*func)(Args... args) : registeredCallbacks)
		{
			func(args...);
		}
	}

	void clear()
	{
		registeredCallbacks.clear();
	}
};