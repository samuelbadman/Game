#pragma once

#include "platform/framework/abstract/platformWindow.h"
#include "sMultiCallback.h"

namespace platformLayer
{
	namespace window
	{
		class platformWindow
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
			// Stores whether the window is in a size move
			bool inSizeMove = false;

			// Callbacks
			sMultiCallback<void(struct platformLayer::window::sResizedEvent&&)> onResizedEventCallback;
			sMultiCallback<void(struct platformLayer::window::sMinimizedEvent&&)> onMinimizedEventCallback;
			sMultiCallback<void(struct platformLayer::window::sMaximizedEvent&&)> onMaximizedEventCallback;
			sMultiCallback<void(struct platformLayer::window::sLostFocusEvent&&)> onLostFocusEventCallback;
			sMultiCallback<void(struct platformLayer::window::sGainedFocusEvent&&)> onGainedFocusEventCallback;
			sMultiCallback<void(struct platformLayer::window::sExitSizeMoveEvent&&)> onExitSizeMoveEventCallback;
			sMultiCallback<void(struct platformLayer::window::sEnterSizeMoveEvent&&)> onEnterSizeMoveEventCallback;
			sMultiCallback<void(struct platformLayer::window::sExitFullScreenEvent&&)> onExitFullScreenEventCallback;
			sMultiCallback<void(struct platformLayer::window::sEnterFullScreenEvent&&)> onEnterFullScreenEventCallback;
			sMultiCallback<void(struct platformLayer::window::sDestroyedEvent&&)> onDestroyedEventCallback;
			sMultiCallback<void(struct platformLayer::window::sClosedEvent&&)> onClosedEventCallback;
			sMultiCallback<void(struct platformLayer::input::sInputEvent&&)> onInputEventCallback;

		public:
			void init(const sWindowDesc& desc);
			void destroy();
			// Returns non-zero if the function fails
			int8_t enterFullScreen();
			// Returns non-zero if the function fails
			int8_t exitFullScreen();
			// Returns non-zero if the function fails
			int8_t setPosition(uint32_t x, uint32_t y);
			// Returns non-zero if the function fails
			int8_t setStyle(eWindowStyle inStyle);
			// Returns true if the window was previously visible and false if the window was previously hidden
			bool show();

			// Returns non-zero if the function fails
			int8_t getClientAreaDimensions(uint32_t& x, uint32_t& y) const;
			// Returns non-zero if the function fails
			int8_t getPosition(uint32_t& x, uint32_t& y) const;
			bool isFullScreen() const { return inFullscreen; }
			HWND getHwnd() const { return hwnd; }
		};
	}
}
