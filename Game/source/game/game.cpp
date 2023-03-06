#include "pch.h"
#include "game.h"
#include "platform/framework/platformConsole.h"
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
#include "math/transform.h"

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
	static constexpr eGraphicsApi graphicsApi = eGraphicsApi::direct3d12;
};

bool game::running = false;
std::shared_ptr<platformWindow> game::window;
std::shared_ptr<platformWindow> game::window2;
std::shared_ptr<graphics> game::graphicsContext;
std::shared_ptr<graphicsSurface> game::surface;
std::shared_ptr<graphicsSurface> game::surface2;
int64_t game::fps = 0;
double game::ms = 0.0;
bool game::graphicsInitialized = false;

sMeshResources game::triangleMeshResources = {};
matrix4x4 game::triangleWorldMatrix;
sRenderData game::triangleRenderData = {};
matrix4x4 game::viewProjectionMatrix;

void game::start()
{
	running = true;

	if (platformLayer::initConsole() != 0)
	{
		platformLayer::messageBoxFatal("game::start: failed to initialize platform console.");
	}

	parseCommandLineArgs();
	initializeWindow();
	initializeGraphics();
	platformLayer::showWindow(window.get());
	platformLayer::showWindow(window2.get());
	initializeAudio();

	// Initialize game loop
	begin();
	
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

		platformLayer::pollOS();
		platformLayer::pollGamepads();

		tick(deltaSeconds);

		while (accumulator > fixedTimeSliceMs)
		{
			fixedTick(sGameSettings::fixedStep);
			accumulator -= fixedTimeSliceMs;
		}

		if (graphicsInitialized)
		{
			render();
		}

		// Update frame timing
		platformLayer::updateTiming(fps, ms);
	}

	shutdownGraphics();
	platformLayer::destroyWindow(window);
	platformLayer::destroyWindow(window2);
}

void game::exit()
{
	running = false;
}

void game::onInputEvent(platformWindow* inWindow, const sInputEvent& evt)
{
}

void game::onWindowMaximized(platformWindow* inWindow, const struct sMaximizedEvent& evt)
{
}

void game::onWindowResized(platformWindow* inWindow, const sResizedEvent& evt)
{
	updateViewProjectionMatrix();

	if (inWindow == window.get())
	{
		graphicsContext->resizeSurface(surface.get(), evt.newClientWidth, evt.newClientHeight);
	}
	else if (inWindow == window2.get())
	{
		graphicsContext->resizeSurface(surface2.get(), evt.newClientWidth, evt.newClientHeight);
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
	wchar_t** argv = platformLayer::getArgcArgv(argc);
	// Todo: Parse arguments
	platformLayer::freeArgv(argv);
}

void game::initializeWindow()
{
	// Get the default display info
	sDisplayDesc defaultDisplayDesc = platformLayer::getInfoForDisplayAtIndex(sGameSettings::defaultDisplayIndex);

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

	platformLayer::createWindow(windowDesc, window);

	windowDesc.windowClassName = L"TestWindow";
	windowDesc.parent = nullptr;
	windowDesc.style = sGameSettings::windowStyle;
	windowDesc.windowTitle = L"Test";
	windowDesc.x = defaultDisplayDesc.topLeftX + sGameSettings::windowPosition[0];
	windowDesc.y = defaultDisplayDesc.topLeftY + sGameSettings::windowPosition[1];
	windowDesc.width = sGameSettings::windowDimensions[0];
	windowDesc.height = sGameSettings::windowDimensions[1];

	platformLayer::createWindow(windowDesc, window2);
}

void game::initializeGraphics()
{
	uint32_t width;
	uint32_t height;
	if (platformLayer::getWindowClientAreaDimensions(window.get(), width, height) != 0)
	{
		platformLayer::messageBoxFatal("initializeGraphics: failed to get window client area dimensions.");
	}
	
	graphics::create(sGameSettings::graphicsApi, graphicsContext);
	if (graphicsContext == nullptr)
	{
		platformLayer::messageBoxFatal("initializeGraphics: failed to create graphics context.");
	}

	graphicsContext->init(false, sGameSettings::enableTripleBuffering ? 3 : 2);
	graphicsContext->createSurface(platformLayer::getWindowHandle(window.get()), width, height, sGameSettings::enableVSync, surface);
	graphicsContext->createSurface(platformLayer::getWindowHandle(window2.get()), width, height, sGameSettings::enableVSync, surface2);

	loadResources();

	graphicsInitialized = true;
}

void game::shutdownGraphics()
{
	graphicsContext->destroySurface(surface);
	graphicsContext->destroySurface(surface2);
	graphicsContext->shutdown();

	graphicsInitialized = false;
}

void game::initializeAudio()
{
	platformLayer::initAudio();
}

void game::loadResources()
{
	const sVertexPos3Norm3Col4UV2 vertices[] = {
		sVertexPos3Norm3Col4UV2(vector3d(-0.5, -0.5, 0.0), vector3d(0.0, 0.0, -1.0), vector4d(0.8, 0.0, 0.0, 1.0), vector2d(0.0, 0.0)), // Bottom left
		sVertexPos3Norm3Col4UV2(vector3d(0.0, 0.5, 0.0), vector3d(0.0, 0.0, -1.0), vector4d(0.0, 0.8, 0.0, 1.0), vector2d(0.5, 1.0)), // Top middle
		sVertexPos3Norm3Col4UV2(vector3d(0.5, -0.5, 0.0), vector3d(0.0, 0.0, -1.0), vector4d(0.0, 0.0, 0.8, 1.0), vector2d(1.0, 0.0)) // Bottom right
	};

	const uint32_t indices[] = { 0, 1, 2 };

	size_t loadVertexCounts[] = { _countof(vertices) };
	const sVertexPos3Norm3Col4UV2 (*loadVertices)[] = { &vertices };
	size_t loadIndexCounts[] = { _countof(indices) };
	const uint32_t (*loadIndices)[] = { &indices };
	sMeshResources* loadOutMeshResources[] = { &triangleMeshResources };
	graphicsContext->loadMeshes(1, loadVertexCounts, loadVertices, loadIndexCounts, loadIndices, loadOutMeshResources);

	transform(vector3d(-1.0, 0.5, 0.0), rotator(0.0, 0.0, 45.0), vector3d(1.0, 1.0, 1.0));
	triangleWorldMatrix = matrix4x4::transpose(matrix4x4::transformation(transform(vector3d(-1.0, 0.5, 0.0), rotator(0.0, 0.0, 45.0), vector3d(1.0, 1.0, 1.0))));
	
	triangleRenderData.pMeshResources = &triangleMeshResources;
	triangleRenderData.pWorldMatrix = &triangleWorldMatrix;
}

void game::begin()
{
}

void game::tick(float deltaSeconds)
{
}

void game::fixedTick(float fixedStep)
{
}

void game::render()
{
	graphicsContext->beginFrame();

	const sRenderData* const renderDatas[] = { &triangleRenderData };
	graphicsSurface* const surfaces[] = { surface.get() };
	graphicsContext->render(_countof(surfaces), surfaces, _countof(renderDatas), renderDatas, &viewProjectionMatrix);

	graphicsSurface* const surfaces2[] = { surface2.get() };
	graphicsContext->render(_countof(surfaces2), surfaces2, 0, nullptr, &viewProjectionMatrix);

	graphicsSurface* const renderedSurfaces[] = { surface.get(), surface2.get() };
	graphicsContext->endFrame(_countof(renderedSurfaces), renderedSurfaces);
}

void game::updateViewProjectionMatrix()
{
	matrix4x4 viewMatrix = matrix4x4::transpose(matrix4x4::view(vector3d(0.0, 0.0, -5.0), rotator(0.0, 0.0, 0.0)));
	uint32_t width, height;
	platformLayer::getWindowClientAreaDimensions(window.get(), width, height);
	static const double orthoZoom = 0.002;
	matrix4x4 projectionMatrix = matrix4x4::transpose(matrix4x4::orthographic(static_cast<double>(width) * orthoZoom, static_cast<double>(height) * orthoZoom, 0.1, 100.0));
	//matrix4x4 projectionMatrix = matrix4x4::transpose(matrix4x4::perspective(45.0, static_cast<double>(width), static_cast<double>(height), 0.1, 100.0));
	viewProjectionMatrix = viewMatrix * projectionMatrix;
}
