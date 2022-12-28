#include "win32Window.h"

bool win32Window::init(const win32WindowInitSettings& settings)
{
	// Store window settings
	windowClassName = settings.windowClassName;
	style = settings.style;

	// Get handle to the executable 
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	const wchar_t* windowClassNameCStr = settings.windowClassName.c_str();

	// Register window class
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;//&InitWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(nullptr);
	windowClass.hIcon = ::LoadIcon(hInstance, IDI_APPLICATION);
	windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassNameCStr;
	windowClass.hIconSm = ::LoadIcon(hInstance, IDI_APPLICATION);

	if (!(RegisterClassExW(&windowClass) > 0))
	{
		// Failed to register window class
		return false;
	}

	// Create the window and store a handle to it
	hwnd = CreateWindowExW(0, windowClassNameCStr, settings.windowTitle.c_str(), styleToDword(settings.style),
		settings.x, settings.y, settings.resX, settings.resY, reinterpret_cast<HWND>(settings.parent), nullptr, hInstance, this);

	if (hwnd == nullptr)
	{
		// Failed to create window
		return false;
	}

	// Register raw input devices
	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = 0;
	rid.hwndTarget = nullptr;

	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
	{
		// Failed to register raw input devices
		return false;
	}

	// Show the window
	ShowWindow(hwnd, SW_SHOW);

	return true;
}

bool win32Window::shutdown()
{
	// Get handle to the executable 
	HINSTANCE hInstance = GetModuleHandleW(nullptr);

	// Unregister the window class
	return UnregisterClassW(windowClassName.c_str(), hInstance);
}

bool win32Window::enterFullScreen()
{
	// Do nothing if the window is currently in full screen
	if (inFullscreen)
	{
		// The window is currently fullscreen
		return false;
	}

	// Store the current window area rect
	if (!GetWindowRect(hwnd, &windowRectOnEnterFullScreen))
	{
		// Failed to get the current window rect
		return false;
	}

	// Retreive info about the monitor the window is on
	HMONITOR hMon{ MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) };
	MONITORINFO monitorInfo = { sizeof(monitorInfo) };
	if (!GetMonitorInfo(hMon, &monitorInfo))
	{
		// Failed to get current monitor info
		return false;
	}

	// Calculate width and height of the monitor
	LONG fWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	LONG fHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	// Update position and size of the window
	if (SetWindowPos(hwnd, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, fWidth, fHeight, SWP_FRAMECHANGED | SWP_NOACTIVATE) == 0)
	{
		return false;
	}

	// Update window style
	if (SetWindowLong(hwnd, GWL_STYLE, 0) == 0)
	{
		return false;
	}

	// Show the window maximized
	ShowWindow(hwnd, SW_MAXIMIZE);

	// Update full screen flag
	inFullscreen = true;

	// Send enter full screen system event 
	//Events::SWindowEnterFullScreenEvent Event = {};
	//Event.pWindow = this;

	//Events::System::SendEventImmediate(std::move(Event));

	return true;
}

bool win32Window::exitFullScreen()
{
	// Do nothing if the window is currently not in full screen
	if (!inFullscreen)
	{
		// The window is not currently fullscreen
		return false;
	}

	// Update position and size of the window
	if (SetWindowPos(hwnd, HWND_TOP, windowRectOnEnterFullScreen.left, windowRectOnEnterFullScreen.top, windowRectOnEnterFullScreen.right - windowRectOnEnterFullScreen.left,
		windowRectOnEnterFullScreen.bottom - windowRectOnEnterFullScreen.top, SWP_FRAMECHANGED | SWP_NOACTIVATE) == 0)
	{
		return false;
	}

	// Update window style
	if (SetWindowLong(hwnd, GWL_STYLE, styleToDword(style)) == 0)
	{
		return false;
	}

	// Show the window
	ShowWindow(hwnd, SW_SHOW);

	// Update full screen flag
	inFullscreen = false;

	// Send exit full screen system event 
	//Events::SWindowExitFullScreenEvent Event = {};
	//Event.pWindow = this;

	//Events::System::SendEventImmediate(std::move(Event));

	return true;
}

bool win32Window::setPosition(int32_t x, int32_t y)
{
	return SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
}

bool win32Window::setStyle(windowStyle inStyle)
{
	style = inStyle;

	// Do not update the window style now if the window is in fullscreen. This will be done when the window leaves fullscreen
	if (!inFullscreen)
	{
		if (SetWindowLong(hwnd, GWL_STYLE, styleToDword(inStyle)) == 0)
		{
			return false;
		}
	}

	return ShowWindow(hwnd, SW_SHOW);
}

void win32Window::getRenderingResolution(int32_t& resX, int32_t& resY) const
{
	RECT clientRect = {};

	if (!GetClientRect(hwnd, &clientRect))
	{
		resX = 0;
		resY = 0;
		return;
	}

	resX = clientRect.right - clientRect.left;
	resY = clientRect.bottom - clientRect.top;
}

void win32Window::getPosition(int32_t& x, int32_t& y) const
{
	RECT windowRect = {};

	if (!GetWindowRect(hwnd, &windowRect))
	{
		x = 0;
		y = 0;
		return;
	}

	x = windowRect.left;
	y = windowRect.top;
}

LRESULT win32Window::onWM_MouseWheel(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_Input(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_LButtonDown(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_RButtonDown(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_MButtonDown(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_LButtonUp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_RButtonUp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_MButtonUp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_KeyDown(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_KeyUp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_EnterSizeMove(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_ExitSizeMove(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_ActivateApp(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_Size(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_Close(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

LRESULT win32Window::onWM_Destroy(win32Window* window, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return LRESULT();
}

DWORD win32Window::styleToDword(const windowStyle style) const
{
	switch (style)
	{
		case windowStyle::windowed: return windowedStyleDword;
		case windowStyle::borderless: return borderlessStyleDword;
		case windowStyle::fixedSize: return fixedSizeStyleDword;
		case windowStyle::noDragSize: return NoDragSizeStyleDword;
		case windowStyle::unset: 
		default: return 0;
	}
}
