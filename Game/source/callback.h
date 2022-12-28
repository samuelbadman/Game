#pragma once

#include <vector>

class callback
{
private:
	std::vector<void(*)()> registeredCallbacks;

public:
	void add(void(*func)());
	void broadcast();
	void clear();
};