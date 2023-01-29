#pragma once

#include "platform/framework/platformWindow.h"

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

	// Stores whether the window is in a size move
	bool inSizeMove = false;

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
	void setInSizeMove(bool inInSizeMove) { inSizeMove = inInSizeMove; }
	bool getInSizeMove() const { return inSizeMove; }

	// Returns non-zero if the function fails
	int8_t getClientAreaDimensions(uint32_t& x, uint32_t& y) const;
	// Returns non-zero if the function fails
	int8_t getPosition(uint32_t& x, uint32_t& y) const;
	bool isFullScreen() const { return inFullscreen; }
	HWND getHwnd() const { return hwnd; }
};
