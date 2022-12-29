#pragma once

#include "callback.h"
#include "Events/Core/closedEvent.h"
#include "Events/Core/destroyedEvent.h"
#include "Events/Core/inputEvent.h"
#include "Events/Core/enterSizeMoveEvent.h"
#include "Events/Core/exitSizeMoveEvent.h"
#include "Events/Core/gainedFocusEvent.h"
#include "Events/Core/lostFocusEvent.h"
#include "Events/Core/maximizedEvent.h"
#include "Events/Core/minimizedEvent.h"
#include "Events/Core/restoredEvent.h"
#include "Events/Core/resizedEvent.h"
#include "Events/Core/enterFullScreenEvent.h"
#include "Events/Core/exitFullScreenEvent.h"

#include <string>
#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Window style enumeration
enum class windowStyle : uint8_t
{
	unset      = 0,
	windowed   = 1,
	borderless = 2,
	noResize   = 3,
	noDragSize = 4
};

// Settings struct to initialize a win32 window
struct win32WindowInitSettings
{
	std::wstring windowClassName = L"";
	std::wstring windowTitle = L"new window";
	int32_t x = 0;
	int32_t y = 0;
	int32_t resX = 1280;
	int32_t resY = 720;
	void* parent = nullptr;
	windowStyle style = windowStyle::windowed;
};

class win32Window
{
private:
	// Fully featured windowed window
	static constexpr DWORD windowedStyleDword = WS_OVERLAPPEDWINDOW;

	// Borderless window
	static constexpr DWORD borderlessStyleDword = WS_POPUPWINDOW;

	// Non resizable windowed window
	static constexpr DWORD noResizeStyleDword = WS_OVERLAPPED | WS_CAPTION |
		WS_SYSMENU | WS_MINIMIZEBOX /*| WS_MAXIMIZEBOX*/;

	// Removes the sizing box restricting drag resizing but retaining the maximize
	// button
	static constexpr DWORD NoDragSizeStyleDword = WS_OVERLAPPEDWINDOW ^ 
		WS_THICKFRAME;

	// The window class name
	std::wstring windowClassName = L"";

	// The current window style when the window is not in fullscreen
	windowStyle style = windowStyle::unset;

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
	callback<const restoredEvent&> onRestored;
	callback<const resizedEvent&> onResized;
	callback<const enterFullScreenEvent&> onEnterFullScreen;
	callback<const exitFullScreenEvent&> onExitFullScreen;

public:
	bool init(const win32WindowInitSettings& settings);
	bool shutdown();
	bool enterFullScreen();
	bool exitFullScreen();
	bool setPosition(int32_t x, int32_t y);
	bool setStyle(windowStyle inStyle);

	void getRenderingResolution(int32_t& resX, int32_t& resY) const;
	void getPosition(int32_t& x, int32_t& y) const;
	bool isFullScreen() const { return inFullscreen; }

	LRESULT onWM_MouseWheel(HWND hwnd, UINT msg, 
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_Input(HWND hwnd, UINT msg, 
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_LButtonDown(HWND hwnd, UINT msg, 
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_RButtonDown(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_MButtonDown(HWND hwnd, UINT msg, 
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_LButtonUp(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_RButtonUp(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_MButtonUp(HWND hwnd, UINT msg, 
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_KeyDown(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_KeyUp(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_EnterSizeMove(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_ExitSizeMove(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_ActivateApp(HWND hwnd, UINT msg, 
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_Size(HWND hwnd, UINT msg, 
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_Close(HWND hwnd, UINT msg, 
		WPARAM wparam, LPARAM lparam);
	LRESULT onWM_Destroy(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);

private:
	DWORD styleToDword(const windowStyle inStyle) const;
};