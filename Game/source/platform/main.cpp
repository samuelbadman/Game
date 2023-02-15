#include "pch.h"
#include "Game/Game.h"

static int initGame()
{
	bool restart = true;
	while (restart)
	{
		restart = game::start();
	}
	return EXIT_SUCCESS;
}

#if defined(PLATFORM_WIN32)

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	return initGame();
}

 #elif defined(0)

#endif // PLATFORM_WIN32
