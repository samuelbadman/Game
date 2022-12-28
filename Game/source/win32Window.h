#pragma once

#include "callback.h"

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
	fixedSize  = 3,
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
	static constexpr DWORD fixedSizeStyleDword = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX /*| WS_MAXIMIZEBOX*/;
	// Removes the sizing box restricting drag resizing but retaining the maximize button
	static constexpr DWORD NoDragSizeStyleDword = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;

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
	callback onClosed;
	callback onDestroyed;

public:
	bool init(const win32WindowInitSettings& settings);
	bool shutdown();
	bool enterFullScreen();
	bool exitFullScreen();
	bool setPosition(int32_t x, int32_t y);
	bool setStyle(windowStyle inStyle);

	void getRenderingResolution(int32_t& resX, int32_t& resY) const;
	void getPosition(int32_t& x, int32_t& y) const;

	LRESULT onWM_MouseWheel(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_Input(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_LButtonDown(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_RButtonDown(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_MButtonDown(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_LButtonUp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_RButtonUp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_MButtonUp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_KeyDown(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_KeyUp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_EnterSizeMove(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_ExitSizeMove(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_ActivateApp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_Size(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_Close(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onWM_Destroy(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	DWORD styleToDword(const windowStyle style) const;
};