#include "win32Window.h"
#include "win32Gamepads.h"
#include "win32Console.h"
#include "stringHelper.h"
#include "game.h"

#include <memory>
#include <chrono>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct gameSettings
{
	// Window settings
	static constexpr windowStyle windowStyle = windowStyle::windowed;
	static constexpr const wchar_t* windowTitle = L"Game";
	static constexpr int32_t windowPosition[2] = { 0, 0 };
	static constexpr int32_t windowDimensions[2] = { 1280, 720 };

	// Rendering settings
	//ERenderingAPI RenderingAPI = ERenderingAPI::DirectX12;
	//Vector4D ClearColor = Vector4D(0.0f, 0.0f, 0.0f, 1.0f);
	//bool VSyncEnabled = true;
	//uint8_t BackBufferCount = 3; // Require renderer restart to apply change

	// Tick settings
	// The time taken in between fixed updates in seconds
	static constexpr double fixedTimeSlice = 0.001;
	// The step size used to step the simulation on each time slice update
	static constexpr float fixedStep = 0.1f;
};

static bool running = true;
static std::unique_ptr<game> gameInstance = nullptr;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, int nCmdShow)
{
	// Initialize console
	const bool consoleInitResult = win32Console::init(2048);
	if (!consoleInitResult)
	{
		MessageBoxA(0, "Failed to init console.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	// Create and initialize window
	std::unique_ptr<win32Window> window = std::make_unique<win32Window>();

	win32WindowInitSettings settings = {};
	settings.windowClassName = L"gameWindow";
	settings.parent = nullptr;
	settings.style = gameSettings::windowStyle;
	settings.windowTitle = gameSettings::windowTitle;
	settings.x = gameSettings::windowPosition[0];
	settings.y = gameSettings::windowPosition[1];
	settings.width = gameSettings::windowDimensions[0];
	settings.height = gameSettings::windowDimensions[1];

	const bool windowInitResult = window->init(settings);
	if (!windowInitResult)
	{
		MessageBoxA(0, "Failed to init window.", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	// Register core event callbacks
	window->onClosed.add([](const closedEvent& event) { running = false; });
	window->onInput.add([](const inputEvent& event) {
		gameInstance->onMouseKeyboardInput(event);
		});
	win32Gamepads::onInput.add([](const inputEvent& event) {
		gameInstance->onGamepadInput(event);
		});

	// Create the game instance
	gameInstance = std::make_unique<game>();

	// Initialize game loop
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
	gameInstance.reset();

	//win32Console::shutdown();
	//window->shutdown();

	return 0;
}