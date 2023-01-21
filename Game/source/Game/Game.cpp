#include "pch.h"
#include "Game.h"
#include "platform/framework/platformWindow.h"
#include "platform/framework/platformWindow.h"
#include "platform/framework/platformDisplay.h"
#include "platform/framework/platformOS.h"
#include "platform/framework/platformGamepad.h"
#include "platform/framework/PlatformAudio.h"
#include "platform/framework/platformTiming.h"
#include "platform/framework/platformMessageBox.h"
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
#include "platform/graphics/direct3D12/direct3d12Graphics.h"

struct sGameSettings
{
	// Window settings
	static constexpr eWindowStyle windowStyle = eWindowStyle::windowed;
	static constexpr const wchar_t* windowTitle = L"Game";
	static constexpr uint32_t windowPosition[2] = { 0, 0 };
	static constexpr uint32_t windowDimensions[2] = { 1280, 720 };
	static constexpr uint32_t defaultDisplayIndex = 0;

	// Tick settings
	// The time taken in between fixed updates in seconds
	static constexpr double fixedTimeSlice = 0.001;
	// The step size used to step the simulation on each time slice update
	static constexpr float fixedStep = 0.1f;
};

bool Game::running = false;
std::shared_ptr<platformWindow> Game::window;

void Game::start()
{
	running = true;

	initializeWindow();
	initializeGraphics();
	initializeAudio();

	// Initialize game loop
	int64_t fps = 0;
	double ms = 0.0;

	double fixedTimeSliceMs = sGameSettings::fixedTimeSlice * 1000.0;
	double accumulator = 0.0;
	std::chrono::time_point previousTime = std::chrono::high_resolution_clock::now();

	while (running)
	{
		// Calculate and accumulate delta time in seconds
		std::chrono::time_point currentTime = std::chrono::high_resolution_clock::now();
		float deltaSeconds = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();
		previousTime = currentTime;
		accumulator += deltaSeconds;

		platformPollOS();
		platformPollGamepads();

		// Todo: Variable tick

		while (accumulator > fixedTimeSliceMs)
		{
			// Todo: Fixed tick(fixed step)
			accumulator -= fixedTimeSliceMs;
		}

		// Todo: Render

		// Update frame timing
		platformUpdateTiming(fps, ms);
	}
}

void Game::exit()
{
	running = false;
}

void Game::onInputEvent(platformWindow* inWindow, const sInputEvent& evt)
{

}

void Game::onWindowMaximized(platformWindow* inWindow, const struct sMaximizedEvent& evt)
{
}

void Game::onWindowResized(platformWindow* inWindow, const sResizedEvent& evt)
{

}

void Game::onWindowMinimized(platformWindow* inWindow, const sMinimizedEvent& evt)
{
}

void Game::onWindowEnterSizeMove(platformWindow* inWindow, const sEnterSizeMoveEvent& evt)
{
}

void Game::onWindowExitSizeMove(platformWindow* inWindow, const sExitSizeMoveEvent& evt)
{
}

void Game::onWindowGainedFocus(platformWindow* inWindow, const sGainedFocusEvent& evt)
{
}

void Game::onWindowLostFocus(platformWindow* inWindow, const sLostFocusEvent& evt)
{
}

void Game::onWindowClosed(platformWindow* inWindow, const sClosedEvent& evt)
{
	if (inWindow == window.get())
	{
		running = false;
	}
}

void Game::onWindowDestroyedEvent(platformWindow* inWindow, const sDestroyedEvent& evt)
{
}

void Game::onWindowEnterFullScreen(platformWindow* inWindow, const sEnterFullScreenEvent& evt)
{
}

void Game::onWindowExitFullScreen(platformWindow* inWindow, const sExitFullScreenEvent& evt)
{
}

void Game::initializeWindow()
{
	// Get the default display info
	sDisplayDesc defaultDisplayDesc = platformGetInfoForDisplayAtIndex(sGameSettings::defaultDisplayIndex);

	// Create and initialize window
	sPlatformWindowDesc windowDesc = {};
	windowDesc.windowClassName = L"GameWindow";
	windowDesc.parent = nullptr;
	windowDesc.style = sGameSettings::windowStyle;
	windowDesc.windowTitle = sGameSettings::windowTitle;
	windowDesc.x = defaultDisplayDesc.topLeftX + sGameSettings::windowPosition[0];
	windowDesc.y = defaultDisplayDesc.topLeftY + sGameSettings::windowPosition[1];
	windowDesc.width = sGameSettings::windowDimensions[0];
	windowDesc.height = sGameSettings::windowDimensions[1];

	platformOpenWindow(windowDesc, window);
}

void Game::initializeGraphics()
{
	uint32_t width;
	uint32_t height;
	if (platformGetWindowClientAreaDimensions(window.get(), width, height) != 0)
	{
		platformMessageBoxFatal("initializeGraphics: failed to get window client area dimensions.");
	}
	
	direct3d12Graphics::init(false, platformGetWindowHandle(window.get()), width, height, 3);
}

void Game::initializeAudio()
{
	platformInitAudio();
}
