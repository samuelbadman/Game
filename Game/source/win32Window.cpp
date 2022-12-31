#include "win32Window.h"
#include "win32InputKeyCode.h"

#include <memory>
#include <assert.h>

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// Get the window pointer instance
	win32Window* const window =
		reinterpret_cast<win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	// Dispatch correct event for the message
	switch (msg)
	{
		case WM_MOUSEWHEEL: return window->onWM_MouseWheel(hwnd, msg, wparam, lparam);
		case WM_INPUT: return window->onWM_Input(hwnd, msg, wparam, lparam);
		case WM_LBUTTONDOWN: return window->onWM_LeftButtonDown(hwnd, msg, wparam, lparam);
		case WM_RBUTTONDOWN: return window->onWM_RightButtonDown(hwnd, msg, wparam, lparam);
		case WM_MBUTTONDOWN: return window->onWM_MiddleButtonDown(hwnd, msg, wparam, lparam);
		case WM_LBUTTONUP: return window->onWM_LeftButtonUp(hwnd, msg, wparam, lparam);
		case WM_RBUTTONUP: return window->onWM_RightButtonUp(hwnd, msg, wparam, lparam);
		case WM_MBUTTONUP: return window->onWM_MiddleButtonUp(hwnd, msg, wparam, lparam);
		case WM_KEYDOWN: return window->onWM_KeyDown(hwnd, msg, wparam, lparam);
		case WM_KEYUP: return window->onWM_KeyUp(hwnd, msg, wparam, lparam);
		case WM_SIZE: return window->onWM_Size(hwnd, msg, wparam, lparam);
		case WM_ENTERSIZEMOVE: return window->onWM_EnterSizeMove(hwnd, msg, wparam, lparam);
		case WM_EXITSIZEMOVE: return window->onWM_ExitSizeMove(hwnd, msg, wparam, lparam);
		case WM_ACTIVATEAPP: return window->onWM_ActivateApp(hwnd, msg, wparam, lparam);
		case WM_CLOSE: return window->onWM_Close(hwnd, msg, wparam, lparam);
		case WM_DESTROY: return window->onWM_Destroy(hwnd, msg, wparam, lparam);
		default: return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

static LRESULT CALLBACK InitWindowProc(HWND hwnd, UINT msg, 
	WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_NCCREATE:
		{
			// Get the create parameters
			const CREATESTRUCTW* const create = 
				reinterpret_cast<CREATESTRUCTW*>(lparam);

			// Get the window instance pointer from the create parameters
			win32Window* const window = reinterpret_cast<win32Window*>(
				create->lpCreateParams);

			// Set the window instance pointer
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 
				reinterpret_cast<LONG_PTR>(window));

			// Set the window procedure pointer
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, 
				reinterpret_cast<LONG_PTR>(WindowProc));

			// Call the window procedure
			return WindowProc(hwnd, msg, wparam, lparam);
		}
	}

	return 0;
}

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
	windowClass.lpfnWndProc = &InitWindowProc;
	windowClass.hInstance = GetModuleHandle(nullptr);
	windowClass.lpszClassName = windowClassNameCStr;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hIcon = ::LoadIcon(hInstance, IDI_APPLICATION);
	windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = NULL;
	windowClass.hIconSm = ::LoadIcon(hInstance, IDI_APPLICATION);

	if (!(RegisterClassExW(&windowClass) > 0))
	{
		// Failed to register window class
		return false;
	}

	// Create the window and store a handle to it
	hwnd = CreateWindowExW(0, windowClassNameCStr, settings.windowTitle.c_str(),
		styleToDword(settings.style),
		settings.x, settings.y, settings.width, settings.height,
		reinterpret_cast<HWND>(settings.parent), nullptr, hInstance, this);

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
	if (SetWindowPos(hwnd, HWND_TOP,
		monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, fWidth, fHeight,
		SWP_FRAMECHANGED | SWP_NOACTIVATE) == 0)
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
	enterFullScreenEvent event = {};
	onEnterFullScreen.broadcast(event);

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
	if (SetWindowPos(hwnd, HWND_TOP, 
		windowRectOnEnterFullScreen.left, windowRectOnEnterFullScreen.top, 
		windowRectOnEnterFullScreen.right - windowRectOnEnterFullScreen.left,
		windowRectOnEnterFullScreen.bottom - windowRectOnEnterFullScreen.top, 
		SWP_FRAMECHANGED | SWP_NOACTIVATE) == 0)
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
	exitFullScreenEvent event = {};
	onExitFullScreen.broadcast(event);

	return true;
}

bool win32Window::setPosition(int32_t x, int32_t y)
{
	return SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
}

bool win32Window::setStyle(windowStyle inStyle)
{
	style = inStyle;

	// Do not update the window style now if the window is in fullscreen. 
	// This will be done when the window leaves fullscreen
	if (!inFullscreen)
	{
		if (SetWindowLong(hwnd, GWL_STYLE, styleToDword(inStyle)) == 0)
		{
			return false;
		}
	}

	return ShowWindow(hwnd, SW_SHOW);
}

void win32Window::getRenderingResolution(int32_t& x, int32_t& y) const
{
	RECT clientRect = {};

	if (!GetClientRect(hwnd, &clientRect))
	{
		x = 0;
		y = 0;
		return;
	}

	x = clientRect.right - clientRect.left;
	y = clientRect.bottom - clientRect.top;
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

LRESULT win32Window::onWM_MouseWheel(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	const auto delta = GET_WHEEL_DELTA_WPARAM(wparam);

	if (delta > 0)
	{
		// Mouse wheel up
		inputEvent event = {};
		event.repeatedKey = false;
		event.input = win32InputKeyCode::Mouse_Wheel_Up;
		event.port = 0;
		event.data = 1.f;
		onInput.broadcast(event);
	}
	else if (delta < 0)
	{
		// Mouse wheel down
		inputEvent event = {};
		event.repeatedKey = false;
		event.input = win32InputKeyCode::Mouse_Wheel_Down;
		event.port = 0;
		event.data = 1.f;
		onInput.broadcast(event);
	}

	return 0;
}

LRESULT win32Window::onWM_Input(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	// Initialize raw data size
	UINT dataSize =  0;

	// Retrieve the size of the raw data
	GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, NULL,
		&dataSize, sizeof(RAWINPUTHEADER));

	// Return if there was no raw data
	if (dataSize == 0)
	{
		return 0;
	}

	// Initialize raw data storage
	std::unique_ptr<BYTE[]> rawData{ std::make_unique<BYTE[]>(dataSize) };

	// Retreive raw data and store it. Return if the retreived data is the not same size
	if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT,
		rawData.get(), &dataSize, sizeof(RAWINPUTHEADER)) != dataSize)
	{
		return 0;
	}

	// Cast byte to raw input
	RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawData.get());

	// Return if the raw input is not from the mouse
	if (raw->header.dwType != RIM_TYPEMOUSE)
	{
		return 0;
	}

	// Raw mouse delta
	inputEvent eventX = {};
	eventX.repeatedKey = false;
	eventX.input = win32InputKeyCode::Mouse_X;
	eventX.port = 0;
	eventX.data = static_cast<float>(raw->data.mouse.lLastX);
	onInput.broadcast(eventX);

	inputEvent eventY = {};
	eventY.repeatedKey = false;
	eventY.input = win32InputKeyCode::Mouse_Y;
	eventY.port = 0;
	eventY.data = static_cast<float>(raw->data.mouse.lLastY);
	onInput.broadcast(eventY);

	return 0;
}

LRESULT win32Window::onWM_LeftButtonDown(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	inputEvent event = {};
	event.repeatedKey = false;
	event.input = win32InputKeyCode::Left_Mouse_Button;
	event.port = 0;
	event.data = 1.f;
	onInput.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_RightButtonDown(HWND hwnd, UINT msg, 
	WPARAM wparam, LPARAM lparam)
{
	inputEvent event = {};
	event.repeatedKey = false;
	event.input = win32InputKeyCode::Right_Mouse_Button;
	event.port = 0;
	event.data = 1.f;
	onInput.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_MiddleButtonDown(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	inputEvent event = {};
	event.repeatedKey = false;
	event.input = win32InputKeyCode::Middle_Mouse_Button;
	event.port = 0;
	event.data = 1.f;
	onInput.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_LeftButtonUp(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	inputEvent event = {};
	event.repeatedKey = false;
	event.input = win32InputKeyCode::Left_Mouse_Button;
	event.port = 0;
	event.data = 0.f;
	onInput.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_RightButtonUp(HWND hwnd, UINT msg, 
	WPARAM wparam, LPARAM lparam)
{
	inputEvent event = {};
	event.repeatedKey = false;
	event.input = win32InputKeyCode::Right_Mouse_Button;
	event.port = 0;
	event.data = 0.f;
	onInput.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_MiddleButtonUp(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	inputEvent event = {};
	event.repeatedKey = false;
	event.input = win32InputKeyCode::Middle_Mouse_Button;
	event.port = 0;
	event.data = 0.f;
	onInput.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_KeyDown(HWND hwnd, UINT msg, 
	WPARAM wparam, LPARAM lparam)
{
	inputEvent event = {};
	event.repeatedKey = static_cast<bool>(lparam & 0x40000000);
	event.input = static_cast<int16_t>(wparam);
	event.port = 0;
	event.data = 1.f;
	onInput.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_KeyUp(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	inputEvent event = {};
	event.repeatedKey = false;
	event.input = static_cast<int16_t>(wparam);
	event.port = 0;
	event.data = 0.f;
	onInput.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_EnterSizeMove(HWND hwnd, UINT msg, 
	WPARAM wparam, LPARAM lparam)
{
	enterSizeMoveEvent event = {};
	onEnterSizeMove.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_ExitSizeMove(HWND hwnd, UINT msg, 
	WPARAM wparam, LPARAM lparam)
{
	exitSizeMoveEvent event = {};
	onExitSizeMove.broadcast(event);
	return 0;
}

LRESULT win32Window::onWM_ActivateApp(HWND hwnd, UINT msg, 
	WPARAM wparam, LPARAM lparam)
{
	if (wparam == TRUE)
	{
		// Received focus
		gainedFocusEvent event = {};
		onGainedFocus.broadcast(event);
		return 0;
	}
	else if (wparam == FALSE)
	{
		// Lost focus
		lostFocusEvent event = {};
		onLostFocus.broadcast(event);
		return 0;
	}

	return 0;
}

LRESULT win32Window::onWM_Size(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	switch (wparam)
	{
		case SIZE_MAXIMIZED:
		{
			// Window maximized
			maximizedEvent maximize = {};
			onMaximized.broadcast(maximize);

			resizedEvent resize = {};
			resize.newResX = LOWORD(lparam);
			resize.newResY = HIWORD(lparam);
			onResized.broadcast(resize);

			return 0;
		}

		case SIZE_MINIMIZED:
		{
			// Window minimized
			minimizedEvent minimize = {};
			onMinimized.broadcast(minimize);

			resizedEvent resize = {};
			resize.newResX = LOWORD(lparam);
			resize.newResY = HIWORD(lparam);
			onResized.broadcast(resize);

			return 0;
		}

		case SIZE_RESTORED:
		{
			// Window restored
			restoredEvent restored = {};
			onRestored.broadcast(restored);

			//resizedEvent resize = {};
			//resize.newResX = LOWORD(lparam);
			//resize.newResY = HIWORD(lparam);
			//onResized.broadcast(resize);

			return 0;
		}
	}

	return 0;
}

LRESULT win32Window::onWM_Close(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam)
{
	closedEvent event = {};
	onClosed.broadcast(event);
	DestroyWindow(hwnd);
	return 0;
}

LRESULT win32Window::onWM_Destroy(HWND hwnd, UINT msg, 
	WPARAM wparam, LPARAM lparam)
{
	destroyedEvent event = {};
	onDestroyed.broadcast(event);
	PostQuitMessage(0);
	return 0;
}

DWORD win32Window::styleToDword(const windowStyle inStyle) const
{
	assert(inStyle != windowStyle::unset);

	switch (inStyle)
	{
		case windowStyle::windowed: return windowedStyleDword;
		case windowStyle::borderless: return borderlessStyleDword;
		case windowStyle::noResize: return noResizeStyleDword;
		case windowStyle::noDragSize: return NoDragSizeStyleDword;
		case windowStyle::unset: 
		default: return 0;
	}
}
