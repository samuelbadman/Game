#include "callback.h"

void callback::add(void(*func)())
{
	registeredCallbacks.push_back(func);
}

void callback::broadcast()
{
	for (void(*func)() : registeredCallbacks)
	{
		func();
	}
}

void callback::clear()
{
	registeredCallbacks.clear();
}
