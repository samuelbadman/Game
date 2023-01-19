#pragma once

// Window style enumeration
enum class eWindowStyle : uint8_t
{
	unknown = 0,
	windowed = 1,
	borderless = 2,
	noResize = 3,
	noDragSize = 4
};

// Settings struct to initialize a platform window
struct sPlatformWindowDesc
{
	std::wstring windowClassName = L"";
	std::wstring windowTitle = L"New window";
	int32_t x = 0;
	int32_t y = 0;
	int32_t width = 1280;
	int32_t height = 720;
	void* parent = nullptr;
	eWindowStyle style = eWindowStyle::windowed;
};

class platformWindow;

extern void platformOpenWindow(const sPlatformWindowDesc& desc, platformWindow*& outPlatformWindow);
extern void platformShutdownWindow(platformWindow* inPlatformWindow);
extern void platformFreeWindow(platformWindow*& outPlatformWindow);
// Returns non-zero if the function fails
extern int8_t platformMakeWindowFullscreen(platformWindow* inPlatformWindow);
// Returns non-zero if the function fails
extern int8_t platformExitWindowFullscreen(platformWindow* inPlatformWindow);
// Returns non-zero if the function fails
extern int8_t platformSetWindowPosition(platformWindow* inPlatformWindow, uint32_t x, uint32_t y);
// Returns non-zero if the function fails
extern int8_t platformSetWindowStyle(platformWindow* inPlatformWindow, eWindowStyle inStyle);
 
// Returns non-zero if the function fails
extern int8_t platformGetWindowClientAreaDimensions(platformWindow* inPlatformWindow, uint32_t& x, uint32_t& y);
// Returns non-zero if the function fails
extern int8_t platformGetWindowPosition(platformWindow* inPlatformWindow, uint32_t& x, uint32_t& y);
extern bool platformIsWindowFullscreen(platformWindow* inPlatformWindow);

