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
struct sWindowDesc
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

namespace platformLayer
{
	namespace window
	{
		extern void createWindow(const sWindowDesc& desc, std::shared_ptr<platformWindow>& outPlatformWindow);
		extern void destroyWindow(std::shared_ptr<platformWindow>& outPlatformWindow);
		// Returns non-zero if the function fails
		extern int8_t makeWindowFullscreen(platformWindow* inPlatformWindow);
		// Returns non-zero if the function fails
		extern int8_t exitWindowFullscreen(platformWindow* inPlatformWindow);
		// Returns non-zero if the function fails
		extern int8_t setWindowPosition(platformWindow* inPlatformWindow, uint32_t x, uint32_t y);
		// Returns non-zero if the function fails
		extern int8_t setWindowStyle(platformWindow* inPlatformWindow, eWindowStyle inStyle);
		// Returns true if the window was previously visible and false if the window was previously hidden
		extern bool showWindow(platformWindow* inPlatformWindow);

		// Returns non-zero if the function fails
		extern int8_t getWindowClientAreaDimensions(platformWindow* inPlatformWindow, uint32_t& x, uint32_t& y);
		// Returns non-zero if the function fails
		extern int8_t getWindowPosition(platformWindow* inPlatformWindow, uint32_t& x, uint32_t& y);
		extern bool isWindowFullscreen(platformWindow* inPlatformWindow);
		extern void* getWindowHandle(platformWindow* inPlatformWindow);
	}
}