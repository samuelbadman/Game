#include "win32Window.h"
#include "display.h"
#include "inputKeyCode.h"

#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static bool running = true;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, int nCmdShow)
{
	std::unique_ptr<win32Window> window = std::make_unique<win32Window>();

	win32WindowInitSettings settings = {};
	settings.windowClassName = L"gameWindow";
	settings.windowTitle = L"Game";

	const bool windowInitResult = window->init(settings);
	if (!windowInitResult)
	{
		MessageBoxA(0, "Failed to init window,", "Error", MB_OK | MB_ICONERROR);
	}

	window->onClosed.add([](const closedEvent& event) { running = false; });

	while (running)
	{
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//window->shutdown();

	return 0;
}