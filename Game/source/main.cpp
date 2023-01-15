#include "pch.h"
#include "platform/win32/win32Console.h"
#include "platform/win32/win32Window.h"
#include "platform/win32/win32Gamepads.h"
#include "platform/win32/win32Display.h"
#include "platform/win32/win32InputKeyCode.h"

struct gameSettings
{
	// Window settings
	static constexpr windowStyle windowStyle = windowStyle::windowed;
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

static bool running = true;
static bool inSizeMove = false;
static std::unique_ptr<win32Window> window = nullptr;

static bool handleAltF4Shortcut(const inputEvent& event)
{
	// Handle alt+f4 shortcut to exit game
	static bool altDown = false;

	if ((event.data == 1.0f) && (!event.repeatedKey))
	{
		if (event.input == win32InputKeyCode::Alt)
		{
			altDown = true;
		}
		else if ((event.input == win32InputKeyCode::F4) && altDown)
		{
			running = false;
		}
	}

	return !running;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, int nCmdShow)
{
	//// Initialize console
	//if (!win32Console::init())
	//{
	//	MessageBoxA(0, "Failed to init console.", "Error", MB_OK | MB_ICONERROR);
	//	return -1;
	//}

	// Get the default display info
	displayInfo defaultDisplayInfo = win32Display::infoForDisplayAtIndex(
		gameSettings::defaultDisplayIndex);

	// Create and initialize window
	window = std::make_unique<win32Window>();

	win32WindowInitSettings windowSettings = {};
	windowSettings.windowClassName = L"GameWindow";
	windowSettings.parent = nullptr;
	windowSettings.style = gameSettings::windowStyle;
	windowSettings.windowTitle = gameSettings::windowTitle;
	windowSettings.x = defaultDisplayInfo.topLeftX + gameSettings::windowPosition[0];
	windowSettings.y = defaultDisplayInfo.topLeftY + gameSettings::windowPosition[1];
	windowSettings.width = gameSettings::windowDimensions[0];
	windowSettings.height = gameSettings::windowDimensions[1];

	if (!window->init(windowSettings))
	{
		MessageBoxA(0, "Failed to init window.", "Error", MB_OK | MB_ICONERROR);
		return -1;
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
		// Todo: Resize renderer
		});

	//window->onGainedFocus.add([](const gainedFocusEvent& event) {  });

	//window->onLostFocus.add([](const lostFocusEvent& event) {  });

	//window->onMaximized.add([](const maximizedEvent& event) {  });

	//window->onMinimized.add([](const minimizedEvent& event) {  });

	window->onResized.add([](const resizedEvent& event) { 
		if (!inSizeMove)
		{
			// Todo: Resize renderer
		}
		});

	//window->onEnterFullScreen.add([](const enterFullScreenEvent& event) {  });

	//window->onExitFullScreen.add([](const exitFullScreenEvent& event) {  });

	window->onInput.add([](const inputEvent& event) {
		if (handleAltF4Shortcut(event))
		{
			return;
		}
		});

	win32Gamepads::onInput.add([](const inputEvent& event) {
		});

	// Initialize game loop
	LARGE_INTEGER startCounter;
	LARGE_INTEGER endCounter;
	LARGE_INTEGER counts;
	LARGE_INTEGER frequency;
	int64_t fps = 0;
	double ms = 0.0;

	QueryPerformanceCounter(&startCounter);
	QueryPerformanceFrequency(&frequency);

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

		// Todo: Variable tick

		// Fixed Tick
		while (accumulator > fixedTimeSliceMs)
		{
			// Fixed tick(fixed step)
			accumulator -= fixedTimeSliceMs;
		}

		// Todo: Render

		// Update frame timing
		QueryPerformanceCounter(&endCounter);

		counts.QuadPart = endCounter.QuadPart - startCounter.QuadPart;

		startCounter = endCounter;

		fps = static_cast<int64_t>(frequency.QuadPart / counts.QuadPart);
		ms = ((1000.0 * static_cast<double>(counts.QuadPart)) / static_cast<double>(frequency.QuadPart));
	}

	return 0;
}