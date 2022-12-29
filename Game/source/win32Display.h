#pragma once

#include <cstdint>
#include <string>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct displayInfo
{
	std::wstring name;
	std::wstring adapterName;
	int32_t topLeftX;
	int32_t topLeftY;
	int32_t resX;
	int32_t resY;
	int32_t verticalRefreshRateHertz;
};

class win32Display
{
public:
	// Retrieves the number of displays connected to the device
	static int32_t displayCount() { return GetSystemMetrics(SM_CMONITORS); }

	// Creates a property structure for the monitor at the specified index. The primary display is at index 0
	static displayInfo infoForDisplayAtIndex(const int32_t displayIndex);
};