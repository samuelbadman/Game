#pragma once

struct displayDesc
{
	std::wstring name = L"";
	std::wstring adapterName = L"";
	uint32_t topLeftX = 0;
	uint32_t topLeftY = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t verticalRefreshRateHertz = 0;
};

class win32Display
{
public:
	// Retrieves the number of displays connected to the device
	static uint32_t displayCount() { return GetSystemMetrics(SM_CMONITORS); }

	// Creates a property structure for the monitor at the specified index. The primary display is at index 0.
	// Index must be smaller than the number of monitors connected to the device
	static displayDesc infoForDisplayAtIndex(const uint32_t displayIndex);
};