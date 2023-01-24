#include "pch.h"

#if defined(PLATFORM_WIN32)

#include "win32Window.h"
#include "platform/framework/platformKeyCodes.h"
#include "platform/framework/platformMessageBox.h"
#include "events/platform/closedEvent.h"
#include "events/platform/destroyedEvent.h"
#include "events/platform/inputEvent.h"
#include "events/platform/enterSizeMoveEvent.h"
#include "events/platform/exitSizeMoveEvent.h"
#include "events/platform/gainedFocusEvent.h"
#include "events/platform/lostFocusEvent.h"
#include "events/platform/maximizedEvent.h"
#include "events/platform/minimizedEvent.h"
#include "events/platform/resizedEvent.h"
#include "events/platform/enterFullScreenEvent.h"
#include "events/platform/exitFullScreenEvent.h"
#include "Game/Game.h"

void platformOpenWindow(const sWindowDesc& desc, std::shared_ptr<platformWindow>& outPlatformWindow)
{
	outPlatformWindow = std::make_shared<platformWindow>();
	outPlatformWindow->init(desc);
}

void platformDestroyWindow(std::shared_ptr<platformWindow>& outPlatformWindow)
{
	outPlatformWindow->destroy();
	outPlatformWindow.reset();
}

int8_t platformMakeWindowFullscreen(platformWindow* inPlatformWindow)
{
	return inPlatformWindow->enterFullScreen();
}

int8_t platformExitWindowFullscreen(platformWindow* inPlatformWindow)
{
	return inPlatformWindow->exitFullScreen();
}

int8_t platformSetWindowPosition(platformWindow* inPlatformWindow, uint32_t x, uint32_t y)
{
	return inPlatformWindow->setPosition(x, y);
}

int8_t platformSetWindowStyle(platformWindow* inPlatformWindow, eWindowStyle inStyle)
{
	return inPlatformWindow->setStyle(inStyle);
}

int8_t platformGetWindowClientAreaDimensions(platformWindow* inPlatformWindow, uint32_t& x, uint32_t& y)
{
	return inPlatformWindow->getClientAreaDimensions(x, y);
}

int8_t platformGetWindowPosition(platformWindow* inPlatformWindow, uint32_t& x, uint32_t& y)
{
	return inPlatformWindow->getPosition(x, y);
}

bool platformIsWindowFullscreen(platformWindow* inPlatformWindow)
{
	return inPlatformWindow->isFullScreen();
}

void* platformGetWindowHandle(platformWindow* inPlatformWindow)
{
	return static_cast<void*>(inPlatformWindow->getHwnd());
}

static DWORD styleToDword(const eWindowStyle inStyle)
{
	switch (inStyle)
	{
		// Fully featured windowed window
	case eWindowStyle::windowed: return WS_OVERLAPPEDWINDOW;
		// Borderless window
	case eWindowStyle::borderless: return WS_POPUPWINDOW;
		// Non resizable windowed window
	case eWindowStyle::noResize: return  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		// Removes the sizing box restricting drag resizing but retaining the maximize button
	case eWindowStyle::noDragSize: return WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;;
	case eWindowStyle::unknown:
	default: return 0;
	}
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// Get the window pointer instance
	if (platformWindow* const window = reinterpret_cast<platformWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
	{
		// Handle message
		switch (msg)
		{
		case WM_MOUSEWHEEL:
		{
			const auto delta = GET_WHEEL_DELTA_WPARAM(wparam);

			if (delta > 0)
			{
				// Mouse wheel up
				sInputEvent evt = {};
				evt.repeatedKey = false;
				evt.input = platformKeyCodes::Mouse_Wheel_Up;
				evt.port = 0;
				evt.data = 1.f;

				Game::onInputEvent(window, evt);
			}
			else if (delta < 0)
			{
				// Mouse wheel down
				sInputEvent evt = {};
				evt.repeatedKey = false;
				evt.input = platformKeyCodes::Mouse_Wheel_Down;
				evt.port = 0;
				evt.data = 1.f;

				Game::onInputEvent(window, evt);
			}

			return 0;
		}

		case WM_INPUT:
		{
			// Initialize raw data size
			UINT dataSize = 0;

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
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT,
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
			sInputEvent evtX = {};
			evtX.repeatedKey = false;
			evtX.input = platformKeyCodes::Mouse_X;
			evtX.port = 0;
			evtX.data = static_cast<float>(raw->data.mouse.lLastX);

			Game::onInputEvent(window, evtX);

			sInputEvent evtY = {};
			evtY.repeatedKey = false;
			evtY.input = platformKeyCodes::Mouse_Y;
			evtY.port = 0;
			evtY.data = static_cast<float>(raw->data.mouse.lLastY);

			Game::onInputEvent(window, evtY);
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformKeyCodes::Left_Mouse_Button;
			evt.port = 0;
			evt.data = 1.f;

			Game::onInputEvent(window, evt);
			return 0;
		}

		case WM_RBUTTONDOWN:
		{
			sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformKeyCodes::Right_Mouse_Button;
			evt.port = 0;
			evt.data = 1.f;

			Game::onInputEvent(window, evt);
			return 0;
		}

		case WM_MBUTTONDOWN:
		{
			sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformKeyCodes::Middle_Mouse_Button;
			evt.port = 0;
			evt.data = 1.f;

			Game::onInputEvent(window, evt);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformKeyCodes::Left_Mouse_Button;
			evt.port = 0;
			evt.data = 0.f;

			Game::onInputEvent(window, evt);
			return 0;
		}

		case WM_RBUTTONUP:
		{
			sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformKeyCodes::Right_Mouse_Button;
			evt.port = 0;
			evt.data = 0.f;

			Game::onInputEvent(window, evt);
			return 0;
		}

		case WM_MBUTTONUP:
		{
			sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformKeyCodes::Middle_Mouse_Button;
			evt.port = 0;
			evt.data = 0.f;

			Game::onInputEvent(window, evt);
			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			// Handle alt+f4 exit shortcut
			bool altBit = false;
			altBit = (lparam & (1 << 29)) != 0;
			if (altBit && (static_cast<int16_t>(wparam) == platformKeyCodes::F4))
			{
				SendMessage(hwnd, WM_CLOSE, 0, 0);
				return 0;
			}

			// Generate input evt
			sInputEvent evt = {};
			evt.repeatedKey = static_cast<bool>(lparam & 0x40000000);
			evt.input = static_cast<int16_t>(wparam);
			evt.port = 0;
			evt.data = 1.f;

			Game::onInputEvent(window, evt);
			return 0;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = static_cast<int16_t>(wparam);
			evt.port = 0;
			evt.data = 0.f;

			Game::onInputEvent(window, evt);
			return 0;
		}

		case WM_SIZE:
		{
			switch (wparam)
			{
			case SIZE_MAXIMIZED:
			{
				// Window maximized
				sMaximizedEvent maximize = {};
				Game::onWindowMaximized(window, maximize);

				sResizedEvent resize = {};
				resize.newClientWidth = LOWORD(lparam);
				resize.newClientHeight = HIWORD(lparam);

				Game::onWindowResized(window, resize);
				return 0;
			}

			case SIZE_MINIMIZED:
			{
				// Window minimized
				sMinimizedEvent minimize = {};

				Game::onWindowMinimized(window, minimize);
				return 0;
			}

			case SIZE_RESTORED:
			{
				return 0;
			}
			}

			return 0;
		}

		case WM_ENTERSIZEMOVE:
		{
			window->setInSizeMove(true);

			sEnterSizeMoveEvent evt = {};

			Game::onWindowEnterSizeMove(window, evt);
			return 0;
		}

		case WM_EXITSIZEMOVE:
		{
			window->setInSizeMove(false);

			sExitSizeMoveEvent evt = {};

			Game::onWindowExitSizeMove(window, evt);

			sResizedEvent resize = {};
			window->getClientAreaDimensions(resize.newClientWidth, resize.newClientHeight);

			Game::onWindowResized(window, resize);
			return 0;
		}

		case WM_ACTIVATEAPP:
		{
			if (wparam == TRUE)
			{
				// Received focus
				sGainedFocusEvent evt = {};

				Game::onWindowGainedFocus(window, evt);
				return 0;
			}
			else if (wparam == FALSE)
			{
				// Lost focus
				sLostFocusEvent evt = {};

				Game::onWindowLostFocus(window, evt);
				return 0;
			}

			return 0;
		}

		case WM_CLOSE:
		{
			sClosedEvent evt = {};

			Game::onWindowClosed(window, evt);
			return 0;
		}

		case WM_DESTROY:
		{
			sDestroyedEvent evt = {};

			Game::onWindowDestroyedEvent(window, evt);
			return 0;
		}
		}
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
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
			platformWindow* const window = reinterpret_cast<platformWindow*>(
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

void platformWindow::init(const sWindowDesc& desc)
{
	// Store window settings
	windowClassName = desc.windowClassName;
	style = desc.style;

	// Get handle to the executable 
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	const wchar_t* windowClassNameCStr = desc.windowClassName.c_str();

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
		platformMessageBoxFatal("win32Window::init failed to register window class.");
	}

	// Create the window and store a handle to it
	if (hwnd = CreateWindowExW(0, windowClassNameCStr, desc.windowTitle.c_str(),
		styleToDword(desc.style),
		desc.x, desc.y, desc.width, desc.height,
		reinterpret_cast<HWND>(desc.parent), nullptr, hInstance, this))
	{
		// Register raw input devices
		RAWINPUTDEVICE rid = {};
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;

		if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
		{
			// Failed to register raw input devices
			platformMessageBoxFatal("win32Window::init failed to register raw input devices.");
		}

		// Show the window
		ShowWindow(hwnd, SW_SHOW);
	}
	else
	{
		// Failed to create window
		platformMessageBoxFatal("win32Window::init failed to create window.");
	}
}

void platformWindow::destroy()
{
	// Destroy the window instance
	if (DestroyWindow(hwnd) == 0)
	{
		platformMessageBoxFatal("win32Window::destroy: failed to destroy window.");
	}

	// Get handle to the executable 
	HINSTANCE hInstance = GetModuleHandleW(nullptr);

	// Unregister the window class
	if (!UnregisterClassW(windowClassName.c_str(), hInstance))
	{
		platformMessageBoxFatal("win32Window::destroy failed to unregister class.");
	}
}

int8_t platformWindow::enterFullScreen()
{
	// Do nothing if the window is currently in full screen
	if (inFullscreen)
	{
		// The window is currently fullscreen
		return 1;
	}

	// Store the current window area rect
	if (!GetWindowRect(hwnd, &windowRectOnEnterFullScreen))
	{
		// Failed to get the current window rect
		return 1;
	}

	// Retreive info about the monitor the window is on
	HMONITOR hMon{ MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) };
	MONITORINFO monitorInfo = { sizeof(monitorInfo) };
	if (!GetMonitorInfo(hMon, &monitorInfo))
	{
		// Failed to get current monitor info
		return 1;
	}

	// Calculate width and height of the monitor
	LONG fWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	LONG fHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	// Update position and size of the window
	if (SetWindowPos(hwnd, HWND_TOP,
		monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, fWidth, fHeight,
		SWP_FRAMECHANGED | SWP_NOACTIVATE) == 0)
	{
		return 1;
	}

	// Update window style
	if (SetWindowLong(hwnd, GWL_STYLE, 0) == 0)
	{
		// SetWindowLong failed
		return 1;
	}

	// Show the window maximized
	ShowWindow(hwnd, SW_MAXIMIZE);

	// Update full screen flag
	inFullscreen = true;

	// Send enter full screen system evt 
	sEnterFullScreenEvent evt = {};

	Game::onWindowEnterFullScreen(this, evt);

	return 0;
}

int8_t platformWindow::exitFullScreen()
{
	// Do nothing if the window is currently not in full screen
	if (!inFullscreen)
	{
		// The window is not currently fullscreen
		return 1;
	}

	// Update position and size of the window
	if (SetWindowPos(hwnd, HWND_TOP, 
		windowRectOnEnterFullScreen.left, windowRectOnEnterFullScreen.top, 
		windowRectOnEnterFullScreen.right - windowRectOnEnterFullScreen.left,
		windowRectOnEnterFullScreen.bottom - windowRectOnEnterFullScreen.top, 
		SWP_FRAMECHANGED | SWP_NOACTIVATE) == 0)
	{
		// SetWindowPos failed
		return 1;
	}

	// Update window style
	if (SetWindowLong(hwnd, GWL_STYLE, styleToDword(style)) == 0)
	{
		// SetWindowLong failed
		return 1;
	}

	// Show the window
	ShowWindow(hwnd, SW_SHOW);

	// Update full screen flag
	inFullscreen = false;

	// Send exit full screen system evt 
	sExitFullScreenEvent evt = {};

	Game::onWindowExitFullScreen(this, evt);

	return 0;
}

int8_t platformWindow::setPosition(uint32_t x, uint32_t y)
{
	if (SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW) == 0)
	{
		// SetWindowPos failed
		return 1;
	}
	return 0;
}

int8_t platformWindow::setStyle(eWindowStyle inStyle)
{
	style = inStyle;

	// Do not update the window style now if the window is in fullscreen. 
	// This will be done when the window leaves fullscreen
	if (!inFullscreen)
	{
		if (SetWindowLong(hwnd, GWL_STYLE, styleToDword(inStyle)) == 0)
		{
			// SetWindowLong failed
			return 1;
		}
	}

	ShowWindow(hwnd, SW_SHOW);

	return 0;
}

int8_t platformWindow::getClientAreaDimensions(uint32_t& x, uint32_t& y) const
{
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect) == 0)
	{
		// GetClientRect failed
		return 1;
	}

	x = clientRect.right - clientRect.left;
	y = clientRect.bottom - clientRect.top;

	return 0;
}

int8_t platformWindow::getPosition(uint32_t& x, uint32_t& y) const
{
	RECT windowRect;
	if (GetWindowRect(hwnd, &windowRect) == 0)
	{
		// GetWindowRect failed
		return 1;
	}

	x = windowRect.left;
	y = windowRect.top;

	return 0;
}

#endif // PLATFORM_WIN32
