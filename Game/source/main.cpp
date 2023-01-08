#include "pch.h"
#include "platform/win32/win32Window.h"
#include "platform/win32/win32Gamepads.h"
#include "log.h"
#include "game.h"
#include "renderer/renderer.h"

struct gameSettings
{
	// Window settings
	static constexpr windowStyle windowStyle = windowStyle::windowed;
	static constexpr const wchar_t* windowTitle = L"Game";
	static constexpr uint32_t windowPosition[2] = { 0, 0 };
	static constexpr uint32_t windowDimensions[2] = { 1280, 720 };
	static constexpr bool startFullScreen = false;
	static constexpr uint32_t defaultDisplayIndex = 0;

	// Rendering settings
	static constexpr rendererPlatform renderingPlatform = rendererPlatform::direct3d12;
	static constexpr bufferingType buffering = bufferingType::tripleBuffering;
	//Vector4D ClearColor = Vector4D(0.0f, 0.0f, 0.0f, 1.0f);
	//bool VSyncEnabled = true;

	// Tick settings
	// The time taken in between fixed updates in seconds
	static constexpr double fixedTimeSlice = 0.001;
	// The step size used to step the simulation on each time slice update
	static constexpr float fixedStep = 0.1f;
};

static bool running = true;
static std::unique_ptr<win32Window> window = nullptr;
static std::unique_ptr<game> gameInstance = nullptr;
static std::unique_ptr<renderDevice> gameRenderDevice = nullptr;
static std::unique_ptr<renderContext> graphicsRenderContext = nullptr;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, int nCmdShow)
{
	// Initialize logging
	LOG_INIT();

	// Create the game instance
	gameInstance = std::make_unique<game>();
	LOG("Created game instance.");

	// Create and initialize window
	window = std::make_unique<win32Window>();

	win32WindowInitSettings windowSettings = {};
	windowSettings.windowClassName = L"gameWindow";
	windowSettings.parent = nullptr;
	windowSettings.style = gameSettings::windowStyle;
	windowSettings.windowTitle = gameSettings::windowTitle;
	windowSettings.x = gameSettings::windowPosition[0];
	windowSettings.y = gameSettings::windowPosition[1];
	windowSettings.width = gameSettings::windowDimensions[0];
	windowSettings.height = gameSettings::windowDimensions[1];

	const bool windowInitResult = window->init(windowSettings);
	if (!windowInitResult)
	{
		MessageBoxA(0, "Failed to init window.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	if (gameSettings::startFullScreen)
	{
		const bool enterFullScreenResult = window->enterFullScreen();
		if (!enterFullScreenResult)
		{
			MessageBoxA(0, "Window failed to enter full screen.", "Error", MB_OK | MB_ICONERROR);
			return -1;
		}
	}

	// Register core event callbacks
	window->onClosed.add([](const closedEvent& event) { running = false; });
	//window->onDestroyed.add([](const destroyedEvent& event) {  });
	//window->onEnterSizeMove.add([](const enterSizeMoveEvent& event) {  });
	//window->onExitSizeMove.add([](const exitSizeMoveEvent& event) {  });
	//window->onGainedFocus.add([](const gainedFocusEvent& event) {  });
	//window->onLostFocus.add([](const lostFocusEvent& event) {  });
	//window->onMaximized.add([](const maximizedEvent& event) {  });
	//window->onMinimized.add([](const minimizedEvent& event) {  });
	//window->onResized.add([](const resizedEvent& event) {  });
	//window->onEnterFullScreen.add([](const enterFullScreenEvent& event) {  });
	//window->onExitFullScreen.add([](const exitFullScreenEvent& event) {  });
	window->onInput.add([](const inputEvent& event) {
		gameInstance->onMouseKeyboardInput(event);
		});
	win32Gamepads::onInput.add([](const inputEvent& event) {
		gameInstance->onGamepadInput(event);
		});
	LOG("Initialized win32 window.");

	// Create and initialize renderer
	// Render device
	gameRenderDevice = renderDevice::create(gameSettings::renderingPlatform);

	renderDeviceInitSettings renderDeviceSettings = {};
	renderDeviceSettings.displayIndex = gameSettings::defaultDisplayIndex;
	renderDeviceSettings.buffering = gameSettings::buffering;
	renderDeviceSettings.graphicsContextSubmissionsPerFrameCount = 1;

	const bool renderDeviceInitResult = gameRenderDevice->init(renderDeviceSettings);
	if (!renderDeviceInitResult)
	{
		MessageBoxA(0, "Failed to initialize render device.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	// Graphics render context
	const bool createGraphicsContextResult = gameRenderDevice->createRenderContext(
		renderCommand::commandContext::graphics, graphicsRenderContext);
	if (!createGraphicsContextResult)
	{
		MessageBoxA(0, "Failed to create graphics render context.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	// Swap chain


	// Initialize game loop
	LOG("Entering game loop.");
	gameInstance->beginPlay();

	double fixedTimeSliceMs = gameSettings::fixedTimeSlice * 1000.0;
	double accumulator = 0.0;
	std::chrono::time_point previousTime = std::chrono::high_resolution_clock::now();

	while (running)
	{
		// Calculate and accumulate delta time in seconds
		std::chrono::time_point currentTime = 
			std::chrono::high_resolution_clock::now();
		double deltaTime = 
			std::chrono::duration<double>(currentTime - previousTime).count();
		previousTime = currentTime;
		accumulator += deltaTime;

		// Dispatch windows messages
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		win32Gamepads::refresh();

		// Fixed Tick
		while (accumulator > fixedTimeSliceMs)
		{
			gameInstance->fixedTick(gameSettings::fixedStep);
			accumulator -= fixedTimeSliceMs;
		}

		// Tick
		gameInstance->tick(static_cast<float>(deltaTime));

		// Render
		gameInstance->render();
	}

	// Destroy the game instance
	//gameInstance.reset();

	// Shutdown and destroy the renderer
	//gameRenderDevice->destroyRenderContext(graphicsRenderContext);
	//LOG("Destroyed graphics render context.");
	//gameRenderDevice->shutdown();
	//gameRenderDevice.reset();
	//LOG("Destroyed render device.");

	// Destroy the window
	//window.reset();
	//LOG("Destroyed win32 window.");

	//LOG_SHUTDOWN();
	//window->shutdown();

	return 0;
}