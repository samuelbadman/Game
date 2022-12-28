#include "win32Window.h"
#include "display.h"

#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool quit = false;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, int nCmdShow)
{
	std::unique_ptr<win32Window> window = std::make_unique<win32Window>();

	win32WindowInitSettings settings = {};
	settings.windowClassName = L"gameWindow";
	settings.windowTitle = L"game";

	window->init(settings);
	window->onClosed.add([]() { quit = true; });

	while (!quit)
	{
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	window->shutdown();

	return 0;
}