#include "pch.h"

#if defined(PLATFORM_WIN32)

#include "win32Window.h"
#include "win32InputKeyCode.h"
#include "win32MessageBox.h"
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

void platformOpenWindow(const sPlatformWindowDesc& desc, platformWindow*& outPlatformWindow)
{
	outPlatformWindow = new platformWindow;
	outPlatformWindow->init(desc);
}

void platformShutdownWindow(platformWindow* inPlatformWindow)
{
	inPlatformWindow->shutdown();
}

void platformFreeWindow(platformWindow*& outPlatformWindow)
{
	delete outPlatformWindow;
	outPlatformWindow = nullptr;
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
				inputEvent event = {};
				event.repeatedKey = false;
				event.input = win32InputKeyCode::Mouse_Wheel_Up;
				event.port = 0;
				event.data = 1.f;

				//Todo: window->onInput.broadcast(event);
			}
			else if (delta < 0)
			{
				// Mouse wheel down
				inputEvent event = {};
				event.repeatedKey = false;
				event.input = win32InputKeyCode::Mouse_Wheel_Down;
				event.port = 0;
				event.data = 1.f;

				// Todo: window->onInput.broadcast(event);
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
			inputEvent eventX = {};
			eventX.repeatedKey = false;
			eventX.input = win32InputKeyCode::Mouse_X;
			eventX.port = 0;
			eventX.data = static_cast<float>(raw->data.mouse.lLastX);

			// Todo: window->onInput.broadcast(eventX);

			inputEvent eventY = {};
			eventY.repeatedKey = false;
			eventY.input = win32InputKeyCode::Mouse_Y;
			eventY.port = 0;
			eventY.data = static_cast<float>(raw->data.mouse.lLastY);

			// Todo: window->onInput.broadcast(eventY);
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			inputEvent event = {};
			event.repeatedKey = false;
			event.input = win32InputKeyCode::Left_Mouse_Button;
			event.port = 0;
			event.data = 1.f;

			// Todo: window->onInput.broadcast(event);
			return 0;
		}

		case WM_RBUTTONDOWN:
		{
			inputEvent event = {};
			event.repeatedKey = false;
			event.input = win32InputKeyCode::Right_Mouse_Button;
			event.port = 0;
			event.data = 1.f;

			// Todo: window->onInput.broadcast(event);
			return 0;
		}

		case WM_MBUTTONDOWN:
		{
			inputEvent event = {};
			event.repeatedKey = false;
			event.input = win32InputKeyCode::Middle_Mouse_Button;
			event.port = 0;
			event.data = 1.f;

			// Todo: window->onInput.broadcast(event);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			inputEvent event = {};
			event.repeatedKey = false;
			event.input = win32InputKeyCode::Left_Mouse_Button;
			event.port = 0;
			event.data = 0.f;

			// Todo: window->onInput.broadcast(event);
			return 0;
		}

		case WM_RBUTTONUP:
		{
			inputEvent event = {};
			event.repeatedKey = false;
			event.input = win32InputKeyCode::Right_Mouse_Button;
			event.port = 0;
			event.data = 0.f;

			// Todo: window->onInput.broadcast(event);
			return 0;
		}

		case WM_MBUTTONUP:
		{
			inputEvent event = {};
			event.repeatedKey = false;
			event.input = win32InputKeyCode::Middle_Mouse_Button;
			event.port = 0;
			event.data = 0.f;

			// Todo: window->onInput.broadcast(event);
			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			inputEvent event = {};
			event.repeatedKey = static_cast<bool>(lparam & 0x40000000);
			event.input = static_cast<int16_t>(wparam);
			event.port = 0;
			event.data = 1.f;

			// Todo: window->onInput.broadcast(event);
			return 0;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			inputEvent event = {};
			event.repeatedKey = false;
			event.input = static_cast<int16_t>(wparam);
			event.port = 0;
			event.data = 0.f;

			// Todo: window->onInput.broadcast(event);
			return 0;
		}

		case WM_SIZE:
		{
			switch (wparam)
			{
			case SIZE_MAXIMIZED:
			{
				// Window maximized
				maximizedEvent maximize = {};

				// Todo: window->onMaximized.broadcast(maximize);

				resizedEvent resize = {};
				resize.newClientWidth = LOWORD(lparam);
				resize.newClientHeight = HIWORD(lparam);

				// Todo: window->onResized.broadcast(resize);
				return 0;
			}

			case SIZE_MINIMIZED:
			{
				// Window minimized
				minimizedEvent minimize = {};

				// Todo: window->onMinimized.broadcast(minimize);
				return 0;
			}

			case SIZE_RESTORED:
			{
				// Window restored
				resizedEvent resize = {};
				resize.newClientWidth = LOWORD(lparam);
				resize.newClientHeight = HIWORD(lparam);

				// Todo: window->onResized.broadcast(resize);
				return 0;
			}
			}

			return 0;
		}

		case WM_ENTERSIZEMOVE:
		{
			enterSizeMoveEvent event = {};

			// Todo: window->onEnterSizeMove.broadcast(event);
			return 0;
		}

		case WM_EXITSIZEMOVE:
		{
			exitSizeMoveEvent event = {};

			// Todo: window->onExitSizeMove.broadcast(event);

			resizedEvent resize = {};
			resize.newClientWidth = LOWORD(lparam);
			resize.newClientHeight = HIWORD(lparam);

			// Todo: window->onResized.broadcast(resize);
			return 0;
		}

		case WM_ACTIVATEAPP:
		{
			if (wparam == TRUE)
			{
				// Received focus
				gainedFocusEvent event = {};

				// Todo: window->onGainedFocus.broadcast(event);
				return 0;
			}
			else if (wparam == FALSE)
			{
				// Lost focus
				lostFocusEvent event = {};

				// Todo: window->onLostFocus.broadcast(event);
				return 0;
			}

			return 0;
		}

		case WM_CLOSE:
		{
			closedEvent event = {};

			// Todo: window->onClosed.broadcast(event);

			DestroyWindow(hwnd);
			return 0;
		}

		case WM_DESTROY:
		{
			destroyedEvent event = {};

			// Todo: window->onDestroyed.broadcast(event);

			PostQuitMessage(0);
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

void platformWindow::init(const sPlatformWindowDesc& desc)
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
		win32MessageBox::messageBoxFatal("win32Window::init failed to register window class.");
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
			win32MessageBox::messageBoxFatal("win32Window::init failed to register raw input devices.");
		}

		// Show the window
		ShowWindow(hwnd, SW_SHOW);
	}
	else
	{
		// Failed to create window
		win32MessageBox::messageBoxFatal("win32Window::init failed to create window.");
	}
}

void platformWindow::shutdown()
{
	// Get handle to the executable 
	HINSTANCE hInstance = GetModuleHandleW(nullptr);

	// Unregister the window class
	if (!UnregisterClassW(windowClassName.c_str(), hInstance))
	{
		win32MessageBox::messageBoxFatal("win32Window::shutdown failed to unregister class.");
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

	// Send enter full screen system event 
	enterFullScreenEvent event = {};

	// Todo: onEnterFullScreen.broadcast(event);

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

	// Send exit full screen system event 
	exitFullScreenEvent event = {};

	// Todo: onExitFullScreen.broadcast(event);

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
