#include "pch.h"
#include "platform/win32/win32Window.h"
#include "platform/win32/win32Gamepads.h"
#include "platform/win32/win32Display.h"
#include "log.h"
#include "stringHelper.h"
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
	static constexpr bool useVSync = false;
	static constexpr float clearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};

	// Tick settings
	// The time taken in between fixed updates in seconds
	static constexpr double fixedTimeSlice = 0.001;
	// The step size used to step the simulation on each time slice update
	static constexpr float fixedStep = 0.1f;
};

static bool running = true;
static bool inSizeMove = false;
static std::unique_ptr<win32Window> window = nullptr;
static std::unique_ptr<game> gameInstance = nullptr;
static std::unique_ptr<renderDevice> gameRenderDevice = nullptr;
static std::unique_ptr<renderContext> graphicsRenderContext = nullptr;
static std::unique_ptr<swapChain> windowSwapChain = nullptr;

static void onRendererResize(const uint32_t newX, const uint32_t newY)
{
	const bool resizeSwapChainResult = gameRenderDevice->resizeSwapChainDimensions(windowSwapChain.get(), newX, newY);
	if (!resizeSwapChainResult)
	{
		MessageBoxA(0, "Failed to resize window swap chain dimesions.", "Error", MB_OK | MB_ICONERROR);
	}
}

// Main thread render
static void render()
{
	// Render into game window
	// Begin a frame
	const uint32_t frameIndex = windowSwapChain->getCurrentBackBufferIndex();
	gameRenderDevice->synchronizeBeginFrame(frameIndex);
	
	// Start recording render contexts across threads here

	renderCommand_beginContext beginContext = {};
	beginContext.frameIndex = frameIndex;
	graphicsRenderContext->submitRenderCommand(beginContext);

	renderCommand_beginFrame beginFrame = {};
	graphicsRenderContext->submitRenderCommand(beginFrame);

	renderCommand_endFrame endFrame = {};
	graphicsRenderContext->submitRenderCommand(endFrame);

	renderCommand_endContext endContext = {};
	graphicsRenderContext->submitRenderCommand(endContext);

	// Wait here for render contexts being recorded across multiple threads before submitting all of them

	// Submit render contexts to render device
	renderContext* graphicsContexts[1] = { graphicsRenderContext.get() };
	gameRenderDevice->submitRenderContexts(renderCommand::commandContext::graphics, 1, graphicsContexts);

	// Present the main window swap chain's back buffer
	windowSwapChain->present(gameSettings::useVSync);
	
	// End the frame
	gameRenderDevice->synchronizeEndFrame(frameIndex);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, int nCmdShow)
{
	// Initialize logging
	LOG_INIT();

	// Get the default display info
	displayInfo defaultDisplayInfo = win32Display::infoForDisplayAtIndex(
		gameSettings::defaultDisplayIndex);

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
	windowSettings.x = defaultDisplayInfo.topLeftX + gameSettings::windowPosition[0];
	windowSettings.y = defaultDisplayInfo.topLeftY + gameSettings::windowPosition[1];
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
	window->onClosed.add([](const closedEvent& event) { 
		running = false; 
		});
	//window->onDestroyed.add([](const destroyedEvent& event) {  });
	window->onEnterSizeMove.add([](const enterSizeMoveEvent& event) {
		inSizeMove = true;
		});
	window->onExitSizeMove.add([](const exitSizeMoveEvent& event) {
		inSizeMove = false;
		onRendererResize(event.newRenderingResolutionX, event.newRenderingResolutionY);
		});
	//window->onGainedFocus.add([](const gainedFocusEvent& event) {  });
	//window->onLostFocus.add([](const lostFocusEvent& event) {  });
	//window->onMaximized.add([](const maximizedEvent& event) {  });
	//window->onMinimized.add([](const minimizedEvent& event) {  });
	window->onResized.add([](const resizedEvent& event) { 
		if (!inSizeMove)
		{
			onRendererResize(event.newRenderingResolutionX, event.newRenderingResolutionY);
		}
		});
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
	renderDeviceSettings.displayConnectedAdapterName = defaultDisplayInfo.adapterName;
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
	swapChainInitSettings swapChainSettings = {};
	swapChainSettings.windowHandle = static_cast<void*>(window->getHwnd());
	window->getRenderingResolution(swapChainSettings.width, swapChainSettings.height);

	const bool swapChainInitResult = gameRenderDevice->createSwapChain(swapChainSettings, windowSwapChain);
	if (!swapChainInitResult)
	{
		MessageBoxA(0, "Failed to create swap chain.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

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
		render();
	}

	return 0;
}