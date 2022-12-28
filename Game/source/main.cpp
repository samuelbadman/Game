#include "win32Window.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	win32Window* window = new win32Window;

	win32WindowInitSettings settings = {};
	settings.windowClassName = L"window";
	settings.windowTitle = L"hello world";

	window->init(settings);

	MessageBoxA(nullptr, "hello world", "game", MB_OK | MB_ICONINFORMATION);

	window->shutdown();
	delete window;

	return 0;
}