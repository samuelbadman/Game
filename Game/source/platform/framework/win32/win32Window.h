#pragma once

#include "platform/framework/abstract/platformWindow.h"

namespace platformLayer
{
	namespace window
	{
		struct platformLayer::window::sResizedEvent;
		struct platformLayer::window::sMinimizedEvent;
		struct platformLayer::window::sMaximizedEvent;
		struct platformLayer::window::sLostFocusEvent;
		struct platformLayer::window::sGainedFocusEvent;
		struct platformLayer::window::sExitSizeMoveEvent;
		struct platformLayer::window::sEnterSizeMoveEvent;
		struct platformLayer::window::sExitFullScreenEvent;
		struct platformLayer::window::sEnterFullScreenEvent;
		struct platformLayer::window::sDestroyedEvent;
		struct platformLayer::window::sClosedEvent;
		struct platformLayer::input::sInputEvent;

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
			std::vector<std::function<void(platformLayer::window::sResizedEvent&&)>> onResizedEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sMinimizedEvent&&)>> onMinimizedEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sMaximizedEvent&&)>> onMaximizedEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sLostFocusEvent&&)>> onLostFocusEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sGainedFocusEvent&&)>> onGainedFocusEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sExitSizeMoveEvent&&)>> onExitSizeMoveEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sEnterSizeMoveEvent&&)>> onEnterSizeMoveEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sExitFullScreenEvent&&)>> onExitFullScreenEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sEnterFullScreenEvent&&)>> onEnterFullScreenEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sDestroyedEvent&&)>> onDestroyedEventCallbacks;
			std::vector<std::function<void(platformLayer::window::sClosedEvent&&)>> onClosedEventCallbacks;
			std::vector<std::function<void(platformLayer::input::sInputEvent&&)>> onInputEventCallbacks;

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

			void broadcast(platformLayer::window::sResizedEvent&& evt) const;
			void broadcast(platformLayer::window::sMinimizedEvent&& evt) const;
			void broadcast(platformLayer::window::sMaximizedEvent&& evt) const;
			void broadcast(platformLayer::window::sLostFocusEvent&& evt) const;
			void broadcast(platformLayer::window::sGainedFocusEvent&& evt) const;
			void broadcast(platformLayer::window::sExitSizeMoveEvent&& evt) const;
			void broadcast(platformLayer::window::sEnterSizeMoveEvent&& evt) const;
			void broadcast(platformLayer::window::sExitFullScreenEvent&& evt) const;
			void broadcast(platformLayer::window::sEnterFullScreenEvent&& evt) const;
			void broadcast(platformLayer::window::sDestroyedEvent&& evt) const;
			void broadcast(platformLayer::window::sClosedEvent&& evt) const;
			void broadcast(platformLayer::input::sInputEvent&& evt) const;

		private:
			template <typename T>
			void broadcastToCallbacks(const std::vector<std::function<void(T&&)>>& callbacks, T&& evt) const
			{
				for (const std::function<void(T&&)>& callback : callbacks)
				{
					callback(std::move(evt));
				}
			}
		};
	}
}
