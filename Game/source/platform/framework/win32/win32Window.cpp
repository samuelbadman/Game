#include "pch.h"

#include "win32Window.h"
#include "platform/framework/abstract/platformKeyCodes.h"
#include "platform/framework/abstract/platformMessageBox.h"
#include "platform/framework/events/sClosedEvent.h"
#include "platform/framework/events/sDestroyedEvent.h"
#include "platform/framework/events/sInputEvent.h"
#include "platform/framework/events/sEnterSizeMoveEvent.h"
#include "platform/framework/events/sExitSizeMoveEvent.h"
#include "platform/framework/events/sGainedFocusEvent.h"
#include "platform/framework/events/sLostFocusEvent.h"
#include "platform/framework/events/sMaximizedEvent.h"
#include "platform/framework/events/sMinimizedEvent.h"
#include "platform/framework/events/sResizedEvent.h"
#include "platform/framework/events/sEnterFullScreenEvent.h"
#include "platform/framework/events/sExitFullScreenEvent.h"

namespace platformLayer
{
	namespace window
	{
		void createWindow(const sWindowDesc& desc, std::shared_ptr<platformWindow>& outPlatformWindow)
		{
			outPlatformWindow = std::make_shared<platformWindow>();
			outPlatformWindow->init(desc);
		}

		void destroyWindow(std::shared_ptr<platformWindow>& outPlatformWindow)
		{
			outPlatformWindow->destroy();
			outPlatformWindow.reset();
		}

		int8_t makeWindowFullscreen(platformWindow* inPlatformWindow)
		{
			return inPlatformWindow->enterFullScreen();
		}

		int8_t exitWindowFullscreen(platformWindow* inPlatformWindow)
		{
			return inPlatformWindow->exitFullScreen();
		}

		int8_t setWindowPosition(platformWindow* inPlatformWindow, uint32_t x, uint32_t y)
		{
			return inPlatformWindow->setPosition(x, y);
		}

		int8_t setWindowStyle(platformWindow* inPlatformWindow, eWindowStyle inStyle)
		{
			return inPlatformWindow->setStyle(inStyle);
		}

		bool showWindow(platformWindow* inPlatformWindow)
		{
			return inPlatformWindow->show();
		}

		int8_t getWindowClientAreaDimensions(platformWindow* inPlatformWindow, uint32_t& x, uint32_t& y)
		{
			return inPlatformWindow->getClientAreaDimensions(x, y);
		}

		int8_t getWindowPosition(platformWindow* inPlatformWindow, uint32_t& x, uint32_t& y)
		{
			return inPlatformWindow->getPosition(x, y);
		}

		bool isWindowFullscreen(platformWindow* inPlatformWindow)
		{
			return inPlatformWindow->isFullScreen();
		}

		void* getWindowHandle(platformWindow* inPlatformWindow)
		{
			return static_cast<void*>(inPlatformWindow->getHwnd());
		}

		void addResizedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sResizedEvent&&)>& inDelegate)
		{
			inPlatformWindow->onResizedEventCallback.bind(inDelegate);
		}

		void addMinimizedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sMinimizedEvent&&)>& inDelegate)
		{
			inPlatformWindow->onMinimizedEventCallback.bind(inDelegate);
		}

		void addMaximizedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sMaximizedEvent&&)>& inDelegate)
		{
			inPlatformWindow->onMaximizedEventCallback.bind(inDelegate);
		}

		void addLostFocusEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sLostFocusEvent&&)>& inDelegate)
		{
			inPlatformWindow->onLostFocusEventCallback.bind(inDelegate);
		}

		void addGainedFocusEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sGainedFocusEvent&&)>& inDelegate)
		{
			inPlatformWindow->onGainedFocusEventCallback.bind(inDelegate);
		}

		void addExitSizeMoveEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sExitSizeMoveEvent&&)>& inDelegate)
		{
			inPlatformWindow->onExitSizeMoveEventCallback.bind(inDelegate);
		}

		void addEnterSizeMoveEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sEnterSizeMoveEvent&&)>& inDelegate)
		{
			inPlatformWindow->onEnterSizeMoveEventCallback.bind(inDelegate);
		}

		void addExitFullScreenEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sExitFullScreenEvent&&)>& inDelegate)
		{
			inPlatformWindow->onExitFullScreenEventCallback.bind(inDelegate);
		}

		void addEnterFullScreenEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sEnterFullScreenEvent&&)>& inDelegate)
		{
			inPlatformWindow->onEnterFullScreenEventCallback.bind(inDelegate);
		}

		void addDestroyedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sDestroyedEvent&&)>& inDelegate)
		{
			inPlatformWindow->onDestroyedEventCallback.bind(inDelegate);
		}

		void addClosedEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::window::sClosedEvent&&)>& inDelegate)
		{
			inPlatformWindow->onClosedEventCallback.bind(inDelegate);
		}

		void addInputEventDelegate(platformWindow* inPlatformWindow, const std::function<void(platformLayer::input::sInputEvent&&)>& inDelegate)
		{
			inPlatformWindow->onInputEventCallback.bind(inDelegate);
		}
	}
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
	if (platformLayer::window::platformWindow* const window = reinterpret_cast<platformLayer::window::platformWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
	{
		// Handle message
		switch (msg)
		{
		case WM_MOUSEWHEEL:
		{
			const short delta = GET_WHEEL_DELTA_WPARAM(wparam);

			if (delta > 0)
			{
				// Mouse wheel up
				platformLayer::input::sInputEvent evt = {};
				evt.repeatedKey = false;
				evt.input = platformLayer::input::keyCodes::Mouse_Wheel_Up;
				evt.port = 0;
				evt.data = 1.f;

				window->onInputEventCallback.broadcast(std::move(evt));
			}
			else if (delta < 0)
			{
				// Mouse wheel down
				platformLayer::input::sInputEvent evt = {};
				evt.repeatedKey = false;
				evt.input = platformLayer::input::keyCodes::Mouse_Wheel_Down;
				evt.port = 0;
				evt.data = 1.f;

				window->onInputEventCallback.broadcast(std::move(evt));
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
			platformLayer::input::sInputEvent evtX = {};
			evtX.repeatedKey = false;
			evtX.input = platformLayer::input::keyCodes::Mouse_X;
			evtX.port = 0;
			evtX.data = static_cast<float>(raw->data.mouse.lLastX);

			window->onInputEventCallback.broadcast(std::move(evtX));

			platformLayer::input::sInputEvent evtY = {};
			evtY.repeatedKey = false;
			evtY.input = platformLayer::input::keyCodes::Mouse_Y;
			evtY.port = 0;
			evtY.data = static_cast<float>(raw->data.mouse.lLastY);

			window->onInputEventCallback.broadcast(std::move(evtY));
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			platformLayer::input::sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformLayer::input::keyCodes::Left_Mouse_Button;
			evt.port = 0;
			evt.data = 1.f;

			window->onInputEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_RBUTTONDOWN:
		{
			platformLayer::input::sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformLayer::input::keyCodes::Right_Mouse_Button;
			evt.port = 0;
			evt.data = 1.f;

			window->onInputEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_MBUTTONDOWN:
		{
			platformLayer::input::sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformLayer::input::keyCodes::Middle_Mouse_Button;
			evt.port = 0;
			evt.data = 1.f;

			window->onInputEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_LBUTTONUP:
		{
			platformLayer::input::sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformLayer::input::keyCodes::Left_Mouse_Button;
			evt.port = 0;
			evt.data = 0.f;

			window->onInputEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_RBUTTONUP:
		{
			platformLayer::input::sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformLayer::input::keyCodes::Right_Mouse_Button;
			evt.port = 0;
			evt.data = 0.f;

			window->onInputEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_MBUTTONUP:
		{
			platformLayer::input::sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = platformLayer::input::keyCodes::Middle_Mouse_Button;
			evt.port = 0;
			evt.data = 0.f;

			window->onInputEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			// Handle alt+f4 exit shortcut
			bool altBit = false;
			altBit = (lparam & (1 << 29)) != 0;
			if (altBit && (static_cast<int16_t>(wparam) == platformLayer::input::keyCodes::F4))
			{
				SendMessage(hwnd, WM_CLOSE, 0, 0);
				return 0;
			}

			// Generate input evt
			platformLayer::input::sInputEvent evt = {};
			evt.repeatedKey = static_cast<bool>(lparam & 0x40000000);
			evt.input = static_cast<int16_t>(wparam);
			evt.port = 0;
			evt.data = 1.f;

			window->onInputEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			platformLayer::input::sInputEvent evt = {};
			evt.repeatedKey = false;
			evt.input = static_cast<int16_t>(wparam);
			evt.port = 0;
			evt.data = 0.f;

			window->onInputEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_SIZE:
		{
			switch (wparam)
			{
			case SIZE_MAXIMIZED:
			{
				// Window maximized
				platformLayer::window::sMaximizedEvent maximize = {};

				window->onMaximizedEventCallback.broadcast(std::move(maximize));

				platformLayer::window::sResizedEvent resize = {};
				resize.newClientWidth = LOWORD(lparam);
				resize.newClientHeight = HIWORD(lparam);

				window->onResizedEventCallback.broadcast(std::move(resize));
				return 0;
			}

			case SIZE_MINIMIZED:
			{
				// Window minimized
				platformLayer::window::sMinimizedEvent minimize = {};

				window->onMinimizedEventCallback.broadcast(std::move(minimize));
				return 0;
			}

			case SIZE_RESTORED:
			{
				// Window restored
				if (!window->inSizeMove)
				{
					platformLayer::window::sResizedEvent resize = {};
					resize.newClientWidth = LOWORD(lparam);
					resize.newClientHeight = HIWORD(lparam);

					window->onResizedEventCallback.broadcast(std::move(resize));
				}

				return 0;
			}
			}

			return 0;
		}

		case WM_ENTERSIZEMOVE:
		{
			window->inSizeMove = true;

			platformLayer::window::sEnterSizeMoveEvent evt = {};

			window->onEnterSizeMoveEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_EXITSIZEMOVE:
		{
			window->inSizeMove = false;

			platformLayer::window::sExitSizeMoveEvent evt = {};

			window->onExitSizeMoveEventCallback.broadcast(std::move(evt));

			platformLayer::window::sResizedEvent resize = {};
			window->getClientAreaDimensions(resize.newClientWidth, resize.newClientHeight);

			window->onResizedEventCallback.broadcast(std::move(resize));
			return 0;
		}

		case WM_ACTIVATEAPP:
		{
			if (wparam == TRUE)
			{
				// Received focus
				platformLayer::window::sGainedFocusEvent evt = {};

				window->onGainedFocusEventCallback.broadcast(std::move(evt));
				return 0;
			}
			else if (wparam == FALSE)
			{
				// Lost focus
				platformLayer::window::sLostFocusEvent evt = {};

				window->onLostFocusEventCallback.broadcast(std::move(evt));
				return 0;
			}

			return 0;
		}

		case WM_CLOSE:
		{
			platformLayer::window::sClosedEvent evt = {};

			window->onClosedEventCallback.broadcast(std::move(evt));
			return 0;
		}

		case WM_DESTROY:
		{
			platformLayer::window::sDestroyedEvent evt = {};

			window->onDestroyedEventCallback.broadcast(std::move(evt));
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
			platformLayer::window::platformWindow* const window = reinterpret_cast<platformLayer::window::platformWindow*>(
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

namespace platformLayer
{
	namespace window
	{
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
				platformLayer::messageBox::showMessageBoxFatal("win32Window::init failed to register window class.");
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
					platformLayer::messageBox::showMessageBoxFatal("win32Window::init failed to register raw input devices.");
				}
			}
			else
			{
				// Failed to create window
				platformLayer::messageBox::showMessageBoxFatal("win32Window::init failed to create window.");
			}
		}

		void platformWindow::destroy()
		{
			// Destroy the window instance
			if (DestroyWindow(hwnd) == 0)
			{
				platformLayer::messageBox::showMessageBoxFatal("win32Window::destroy: failed to destroy window.");
			}

			// Get handle to the executable 
			HINSTANCE hInstance = GetModuleHandleW(nullptr);

			// Unregister the window class
			if (!UnregisterClassW(windowClassName.c_str(), hInstance))
			{
				platformLayer::messageBox::showMessageBoxFatal("win32Window::destroy failed to unregister class.");
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
			platformLayer::window::sEnterFullScreenEvent evt = {};

			onEnterFullScreenEventCallback.broadcast(std::move(evt));
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
			platformLayer::window::sExitFullScreenEvent evt = {};

			onExitFullScreenEventCallback.broadcast(std::move(evt));
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

		bool platformWindow::show()
		{
			// Show the window
			return (ShowWindow(hwnd, SW_SHOW) != 0);
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
	}
}