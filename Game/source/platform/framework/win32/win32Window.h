#pragma once

#include "callback.h"
#include "events/core/closedEvent.h"
#include "events/core/destroyedEvent.h"
#include "events/core/inputEvent.h"
#include "events/core/enterSizeMoveEvent.h"
#include "events/core/exitSizeMoveEvent.h"
#include "events/core/gainedFocusEvent.h"
#include "events/core/lostFocusEvent.h"
#include "events/core/maximizedEvent.h"
#include "events/core/minimizedEvent.h"
#include "events/core/resizedEvent.h"
#include "events/core/enterFullScreenEvent.h"
#include "events/core/exitFullScreenEvent.h"

// Window style enumeration
enum class eWindowStyle : uint8_t
{
	unknown      = 0,
	windowed     = 1,
	borderless   = 2,
	noResize     = 3,
	noDragSize   = 4
};

// Settings struct to initialize a win32 window
struct win32WindowInitDesc
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

class win32Window
{
private:
	// The window class name
	std::wstring windowClassName = L"";

	// The current window style when the window is not in fullscreen
	eWindowStyle style = eWindowStyle::unknown;

	// The window handle
	HWND hwnd = nullptr;

	// The current window rect when the window is enters fullscreen
	RECT windowRectOnEnterFullScreen = {};

	// Stores whether the window is in fullscreen
	bool inFullscreen = false;

public:
	callback<const closedEvent&> onClosed;
	callback<const destroyedEvent&> onDestroyed;
	callback<const inputEvent&> onInput;
	callback<const enterSizeMoveEvent&> onEnterSizeMove;
	callback<const exitSizeMoveEvent&> onExitSizeMove;
	callback<const gainedFocusEvent&> onGainedFocus;
	callback<const lostFocusEvent&> onLostFocus;
	callback<const maximizedEvent&> onMaximized;
	callback<const minimizedEvent&> onMinimized;
	callback<const resizedEvent&> onResized;
	callback<const enterFullScreenEvent&> onEnterFullScreen;
	callback<const exitFullScreenEvent&> onExitFullScreen;

public:
	void init(const win32WindowInitDesc& desc);
	void shutdown();
	// Returns non-zero if the function fails
	int8_t enterFullScreen();
	// Returns non-zero if the function fails
	int8_t exitFullScreen();
	// Returns non-zero if the function fails
	int8_t setPosition(uint32_t x, uint32_t y);
	// Returns non-zero if the function fails
	int8_t setStyle(eWindowStyle inStyle);

	// Returns non-zero if the function fails
	int8_t getClientAreaDimensions(uint32_t& x, uint32_t& y) const;
	// Returns non-zero if the function fails
	int8_t getPosition(uint32_t& x, uint32_t& y) const;
	bool isFullScreen() const { return inFullscreen; }
	HWND getHwnd() const { return hwnd; }
};