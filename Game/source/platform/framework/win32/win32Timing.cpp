#include "pch.h"

#if defined(PLATFORM_WIN32)

void platformUpdateTiming(int64_t& outFps, double& outMs)
{
	static LARGE_INTEGER startCounter;
	static LARGE_INTEGER endCounter;
	static LARGE_INTEGER frequency;
	static LARGE_INTEGER counts;

	static BOOL queryPerfStartCounter = [&]() { return QueryPerformanceCounter(&startCounter); } ();
	static BOOL queryPerfFrequency = [&]() { return QueryPerformanceFrequency(&frequency); } ();

	QueryPerformanceCounter(&endCounter);
	counts.QuadPart = endCounter.QuadPart - startCounter.QuadPart;
	startCounter = endCounter;

	outFps = static_cast<int64_t>(frequency.QuadPart / counts.QuadPart);
	outMs = ((1000.0 * static_cast<double>(counts.QuadPart)) / static_cast<double>(frequency.QuadPart));
}

#endif // PLATFORM_WIN32
