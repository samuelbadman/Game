#include "pch.h"
#include "platform/framework/platformCommandLine.h"
#include "platform/framework/platformWindow.h"
#include "platform/framework/platformDisplay.h"
#include "platform/framework/platformMessages.h"
#include "platform/framework/platformGamepad.h"
#include "platform/framework/PlatformAudio.h"
#include "platform/framework/platformTiming.h"

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

static bool running = true;
static bool inSizeMove = false;
static platformWindow* window = nullptr;

static void processCommandLineArguments()
{
	int argc;
	wchar_t** argv = platformGetCommandLineArguments(argc);
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

	platformFreeCommandLineArguments(argv);
}

static void initializeWindow()
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

static void initializeGraphics()
{
	//uint32_t width;
	//uint32_t height;
	//if (window->getClientAreaDimensions(width, height) != 0)
	//{
	//	win32MessageBox::messageBoxFatal("initializeGraphics: failed to get window client area dimensions.");
	//}

	//direct3d12Graphics::init(false, window->getHwnd(), width, height, 3);
}

static void initializeAudio()
{
	platformInitAudio();
}

static int gameMain()
{
	processCommandLineArguments();
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

		platformDispatchMessages();
		platformRefreshGamepads();

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

	return EXIT_SUCCESS;
}

#if defined(PLATFORM_WIN32)
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	return gameMain();
}
 #elif defined(0)
#endif // PLATFORM_WIN32
