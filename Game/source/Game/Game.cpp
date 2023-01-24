#include "pch.h"
#include "game.h"
#include "platform/framework/platformCommandLine.h"
#include "platform/framework/platformWindow.h"
#include "platform/framework/platformDisplay.h"
#include "platform/framework/platformOS.h"
#include "platform/framework/platformGamepad.h"
#include "platform/framework/PlatformAudio.h"
#include "platform/framework/platformTiming.h"
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

	// Render settings
	static constexpr bool enableVSync = false;
};

bool game::running = false;
std::shared_ptr<platformWindow> game::window;
std::shared_ptr<graphicsSurface> game::surface;
int64_t game::fps = 0;
double game::ms = 0.0;

void game::start()
{
	running = true;

	parseCommandLineArgs();
	initializeWindow();
	initializeGraphics();
	initializeAudio();

	// Initialize game loop
	double fixedTimeSliceMs = sGameSettings::fixedTimeSlice * 1000.0;
	double accumulator = 0.0;
	std::chrono::time_point previousTime = std::chrono::high_resolution_clock::now();

	while (running)
	{
		// Calculate and accumulate delta time in seconds
		std::chrono::time_point currentTime = std::chrono::high_resolution_clock::now();
		const float deltaSeconds = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();
		previousTime = currentTime;
		accumulator += deltaSeconds;

		platformPollOS();
		platformPollGamepads();

		tick(deltaSeconds);

		while (accumulator > fixedTimeSliceMs)
		{
			fixedTick(sGameSettings::fixedStep);
			accumulator -= fixedTimeSliceMs;
		}

		render();

		// Update frame timing
		platformUpdateTiming(fps, ms);
	}

	shutdownGraphics();
}

void game::exit()
{
	running = false;
	platformDestroyWindow(window);
}

void game::onInputEvent(platformWindow* inWindow, const sInputEvent& evt)
{

}

void game::onWindowMaximized(platformWindow* inWindow, const struct sMaximizedEvent& evt)
{
}

void game::onWindowResized(platformWindow* inWindow, const sResizedEvent& evt)
{
	if (inWindow == window.get())
	{
		direct3d12Graphics::resizeSurface(surface.get(), evt.newClientWidth, evt.newClientHeight);
	}
}

void game::onWindowMinimized(platformWindow* inWindow, const sMinimizedEvent& evt)
{
}

void game::onWindowEnterSizeMove(platformWindow* inWindow, const sEnterSizeMoveEvent& evt)
{
}

void game::onWindowExitSizeMove(platformWindow* inWindow, const sExitSizeMoveEvent& evt)
{
}

void game::onWindowGainedFocus(platformWindow* inWindow, const sGainedFocusEvent& evt)
{
}

void game::onWindowLostFocus(platformWindow* inWindow, const sLostFocusEvent& evt)
{
}

void game::onWindowClosed(platformWindow* inWindow, const sClosedEvent& evt)
{
	if (inWindow == window.get())
	{
		exit();
	}
}

void game::onWindowDestroyedEvent(platformWindow* inWindow, const sDestroyedEvent& evt)
{

}

void game::onWindowEnterFullScreen(platformWindow* inWindow, const sEnterFullScreenEvent& evt)
{
}

void game::onWindowExitFullScreen(platformWindow* inWindow, const sExitFullScreenEvent& evt)
{
}

void game::parseCommandLineArgs()
{
	int32_t argc;
	wchar_t** argv = platformGetArgcArgv(argc);
	// Todo: Parse arguments
	platformFreeArgv(argv);
}

void game::initializeWindow()
{
	// Get the default display info
	sDisplayDesc defaultDisplayDesc = platformGetInfoForDisplayAtIndex(sGameSettings::defaultDisplayIndex);

	// Create and initialize window
	sWindowDesc windowDesc = {};
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

void game::initializeGraphics()
{
	uint32_t width;
	uint32_t height;
	if (platformGetWindowClientAreaDimensions(window.get(), width, height) != 0)
	{
		platformMessageBoxFatal("initializeGraphics: failed to get window client area dimensions.");
	}
	
	direct3d12Graphics::init(false, 3);
	direct3d12Graphics::createSurface(platformGetWindowHandle(window.get()), width, height, surface);
}

void game::shutdownGraphics()
{
	direct3d12Graphics::shutdown();
}

void game::initializeAudio()
{
	platformInitAudio();
}

void game::tick(float deltaSeconds)
{
}

void game::fixedTick(float fixedStep)
{
}

void game::render()
{
	graphicsSurface* const surfaces[1] = { surface.get() };
	direct3d12Graphics::render(_countof(surfaces), surfaces, sGameSettings::enableVSync);
}
