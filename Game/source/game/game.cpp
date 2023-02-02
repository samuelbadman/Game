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
#include "platform/events/closedEvent.h"
#include "platform/events/destroyedEvent.h"
#include "platform/events/inputEvent.h"
#include "platform/events/enterSizeMoveEvent.h"
#include "platform/events/exitSizeMoveEvent.h"
#include "platform/events/gainedFocusEvent.h"
#include "platform/events/lostFocusEvent.h"
#include "platform/events/maximizedEvent.h"
#include "platform/events/minimizedEvent.h"
#include "platform/events/resizedEvent.h"
#include "platform/events/enterFullScreenEvent.h"
#include "platform/events/exitFullScreenEvent.h"
#include "platform/graphics/graphics.h"

#include "platform/graphics/vertexPos3Norm3Col4UV2.h"
#include "math/vector3d.h"
#include "math/vector4d.h"
#include "math/vector2d.h"

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
	static constexpr bool enableTripleBuffering = false;
};

bool game::running = false;
std::shared_ptr<platformWindow> game::window;
std::shared_ptr<graphicsSurface> game::surface;
int64_t game::fps = 0;
double game::ms = 0.0;

sMeshResources game::triangleMeshResources = {};

void game::start()
{
	running = true;

	parseCommandLineArgs();
	initializeWindow();
	initializeGraphics();
	initializeAudio();

	loadResources();

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
		graphicsResizeSurface(surface.get(), evt.newClientWidth, evt.newClientHeight);
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
	
	graphicsInit(eGraphicsApi::direct3d12, false, sGameSettings::enableTripleBuffering ? 3 : 2);
	graphicsCreateSurface(platformGetWindowHandle(window.get()), width, height, surface);
}

void game::shutdownGraphics()
{
	graphicsShutdown();
}

void game::initializeAudio()
{
	platformInitAudio();
}

void game::loadResources()
{
	const sVertexPos3Norm3Col4UV2 vertices[] = {
		sVertexPos3Norm3Col4UV2(vector3d(-0.5, -0.5, 0.0), vector3d(0.0, 0.0, -1.0), vector4d(0.8, 0.0, 0.0, 1.0), vector2d(0.0, 0.0)), // Bottom left
		sVertexPos3Norm3Col4UV2(vector3d(0.0, 0.5, 0.0), vector3d(0.0, 0.0, -1.0), vector4d(0.0, 0.8, 0.0, 1.0), vector2d(0.5, 1.0)), // Top middle
		sVertexPos3Norm3Col4UV2(vector3d(0.5, -0.5, 0.0), vector3d(0.0, 0.0, -1.0), vector4d(0.0, 0.0, 0.8, 1.0), vector2d(1.0, 0.0)) // Bottom right
	};

	const uint32_t indices[] = { 0, 1, 2 };

	graphicsLoadMesh(_countof(vertices), vertices, _countof(indices), indices, triangleMeshResources);
}

void game::tick(float deltaSeconds)
{
}

void game::fixedTick(float fixedStep)
{
}

void game::render()
{
	const graphicsSurface* const surfaces[] = { surface.get() };
	const sMeshResources* const meshes[] = { &triangleMeshResources };
	graphicsRender(_countof(surfaces), surfaces, sGameSettings::enableVSync, _countof(meshes), meshes);
}
