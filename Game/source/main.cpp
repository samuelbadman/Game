#include "win32Window.h"
#include "display.h"
#include "inputKeyCode.h"

#include <memory>
#include <chrono>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct gameSettings
{
	// Window settings
	static constexpr windowStyle windowStyle = windowStyle::windowed;

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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, int nCmdShow)
{
	std::unique_ptr<win32Window> window = std::make_unique<win32Window>();

	win32WindowInitSettings settings = {};
	settings.windowClassName = L"gameWindow";
	settings.windowTitle = L"Game";
	settings.style = gameSettings::windowStyle;

	const bool windowInitResult = window->init(settings);
	if (!windowInitResult)
	{
		MessageBoxA(0, "Failed to init window,", "Error", MB_OK | MB_ICONERROR);
	}

	window->onClosed.add([](const closedEvent& event) { running = false; });

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

		// Fixed Tick
		while (accumulator > fixedTimeSliceMs)
		{
			//fixedTick(gameSettings::fixedStep);
			accumulator -= fixedTimeSliceMs;
		}

		// Tick
		//tick(static_cast<float>(deltaTime));

		// Render
		//render();
	}

	//window->shutdown();

	return 0;
}