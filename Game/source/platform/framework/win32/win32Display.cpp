#include "pch.h"
#include "win32Display.h"

sDisplayDesc win32Display::infoForDisplayAtIndex(const uint32_t displayIndex)
{
	assert(displayIndex < displayCount());

	// Initialize property structure
	sDisplayDesc info = {};

	// Initialize the DISPLAY_DEVICE structure
	DISPLAY_DEVICE displayDevice = {};
	displayDevice.cb = sizeof(DISPLAY_DEVICE);

	// Enumerate display device
	EnumDisplayDevices(NULL, displayIndex, &displayDevice,
		EDD_GET_DEVICE_INTERFACE_NAME);

	// Enumerate the display device's settings
	DEVMODEW displaySettings = {};
	EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS,
		&displaySettings);

	// Fill in hardware properties info with the device's details
	info.name = displayDevice.DeviceName;
	info.adapterName = displayDevice.DeviceString;
	info.topLeftX = displaySettings.dmPosition.x;
	info.topLeftY = displaySettings.dmPosition.y;
	info.width = displaySettings.dmPelsWidth;
	info.height = displaySettings.dmPelsHeight;
	info.verticalRefreshRateHertz = displaySettings.dmDisplayFrequency;

	// Return the hardware property info
	return info;
}
