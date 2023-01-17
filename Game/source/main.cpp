#include "pch.h"
#include "platform/framework/win32/win32MessageBox.h"
#include "platform/framework/win32/win32Console.h"
#include "platform/framework/win32/win32Window.h"
#include "platform/framework/win32/win32Display.h"
#include "platform/framework/win32/win32InputKeyCode.h"
#include "platform/gamepad/xInput/xInputGamepad.h"
#include "platform/graphics/direct3D12/direct3d12Graphics.h"
#include "platform/audio/xAudio2/xAudio2Audio.h"

struct gameSettings
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

static bool running = true;
static bool inSizeMove = false;
static std::unique_ptr<win32Window> window = nullptr;

static void processCommandLineArguments()
{
	int argc;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	for (size_t i = 0; i < argc; ++i)
	{
		if (wcscmp(argv[i], L"") == 0)
		{
		}

		//if (::wcscmp(argv[i], L"-w") == 0 || ::wcscmp(argv[i], L"--width") == 0)
		//{
		//	g_ClientWidth = ::wcstol(argv[++i], nullptr, 10);
		//}
		//if (::wcscmp(argv[i], L"-h") == 0 || ::wcscmp(argv[i], L"--height") == 0)
		//{
		//	g_ClientHeight = ::wcstol(argv[++i], nullptr, 10);
		//}
		//if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0)
		//{
		//	g_UseWarp = true;
		//}
	}

	LocalFree(argv);
}

static bool initializeWindow()
{
	// Get the default display info
	displayDesc defaultDisplayDesc = win32Display::infoForDisplayAtIndex(
		gameSettings::defaultDisplayIndex);

	// Create and initialize window
	window = std::make_unique<win32Window>();

	win32WindowInitDesc windowDesc = {};
	windowDesc.windowClassName = L"GameWindow";
	windowDesc.parent = nullptr;
	windowDesc.style = gameSettings::windowStyle;
	windowDesc.windowTitle = gameSettings::windowTitle;
	windowDesc.x = defaultDisplayDesc.topLeftX + gameSettings::windowPosition[0];
	windowDesc.y = defaultDisplayDesc.topLeftY + gameSettings::windowPosition[1];
	windowDesc.width = gameSettings::windowDimensions[0];
	windowDesc.height = gameSettings::windowDimensions[1];

	if (!window->init(windowDesc))
	{
		return false;
	}

	// Register core event callbacks
	window->onClosed.add([](const closedEvent& event) {
		running = false;
		});

	window->onEnterSizeMove.add([](const enterSizeMoveEvent& event) {
		inSizeMove = true;
		});

	window->onExitSizeMove.add([](const exitSizeMoveEvent& event) {
		inSizeMove = false;
		});

	window->onResized.add([](const resizedEvent& event) {
		if (!inSizeMove)
		{
			// Todo: Resize swap chain and render resources
		}
		});

	//window->onDestroyed.add([](const destroyedEvent& event) {  });

	//window->onGainedFocus.add([](const gainedFocusEvent& event) {  });

	//window->onLostFocus.add([](const lostFocusEvent& event) {  });

	//window->onMaximized.add([](const maximizedEvent& event) {  });

	//window->onMinimized.add([](const minimizedEvent& event) {  });

	//window->onEnterFullScreen.add([](const enterFullScreenEvent& event) {  });

	//window->onExitFullScreen.add([](const exitFullScreenEvent& event) {  });

	static auto altF4Down = [](const inputEvent& event)
	{
		// Handle alt+f4 shortcut to exit game
		static bool altDown = false;

		if ((!event.repeatedKey) && (event.data == 1.0f))
		{
			if (event.input == win32InputKeyCode::Alt)
			{
				altDown = true;
			}
			else if (altDown && (event.input == win32InputKeyCode::F4))
			{
				return true;
			}
		}

		return false;
	};

	window->onInput.add([](const inputEvent& event) {
		if (altF4Down(event))
		{
			running = false;
		}
		});

	return true;
}

static void initializeGamepadInput()
{
	xInputGamepad::onInput.add([](const inputEvent& event) {
		});
}

static bool initializeGraphics()
{
	return direct3d12Graphics::init();
}

static bool initializeAudio()
{
	return xAudio2Audio::init();
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	// Initialize console
	if (!win32Console::init())
	{
		win32MessageBox::messageBox(eMessageLevel::error, "Failed to initialize console.");
		return EXIT_FAILURE;
	}

	// Process command line arguments
	processCommandLineArguments();

	// Initialize window
	if (!initializeWindow())
	{
		win32MessageBox::messageBox(eMessageLevel::error, "Failed to initialize window.");
		return EXIT_FAILURE;
	}

	// Initialize gamepad input
	initializeGamepadInput();

	// Initialize graphics
	if (!initializeGraphics())
	{
		win32MessageBox::messageBox(eMessageLevel::error, "Failed to initialize graphics.");
		return EXIT_FAILURE;
	}

	// Initialize audio
	if (!initializeAudio())
	{
		win32MessageBox::messageBox(eMessageLevel::error, "Failed to initialize audio.");
		return EXIT_FAILURE;
	}

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
		std::chrono::time_point currentTime = std::chrono::high_resolution_clock::now();
		float deltaSeconds = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();
		previousTime = currentTime;
		accumulator += deltaSeconds;

		// Dispatch windows messages
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		xInputGamepad::refreshUsers();

		// Todo: Variable tick

		while (accumulator > fixedTimeSliceMs)
		{
			// Todo: Fixed tick(fixed step)
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

	return EXIT_SUCCESS;
}