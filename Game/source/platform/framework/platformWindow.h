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

namespace platformLayer
{
	namespace input
	{
		struct sInputEvent;
	}

	namespace window
	{
		class platformWindow;

		struct sResizedEvent;
		struct sMinimizedEvent;
		struct sMaximizedEvent;
		struct sLostFocusEvent;
		struct sGainedFocusEvent;
		struct sExitSizeMoveEvent;
		struct sEnterSizeMoveEvent;
		struct sExitFullScreenEvent;
		struct sEnterFullScreenEvent;
		struct sDestroyedEvent;
		struct sClosedEvent;

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

		// Bind event delegate to inPlatformWindow's event callback
		extern void addResizedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sResizedEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addMinimizedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sMinimizedEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addMaximizedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sMaximizedEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addLostFocusEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sLostFocusEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addGainedFocusEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sGainedFocusEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addExitSizeMoveEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sExitSizeMoveEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addEnterSizeMoveEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sEnterSizeMoveEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addExitFullScreenEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sExitFullScreenEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addEnterFullScreenEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sEnterFullScreenEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addDestroyedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sDestroyedEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addClosedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sClosedEvent&&)>& inDelegate);
		// Bind event delegate to inPlatformWindow's event callback
		extern void addInputEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::input::sInputEvent&&)>& inDelegate);
	}
}