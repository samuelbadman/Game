#include "pch.h"
#include "Game.h"

static int initGame()
{
	// Todo: Parse command line arguments for additional inialize settings for the game. e.g. Additional debug tools. 
	Game::start();
	return EXIT_SUCCESS;
}

#if defined(PLATFORM_WIN32)

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	return initGame();
}

 #elif defined(0)

#endif // PLATFORM_WIN32
