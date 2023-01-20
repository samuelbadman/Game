#include "pch.h"

#if defined(PLATFORM_WIN32)

void platformPollOS()
{
	// Dispatch windows messages
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

#endif // PLATFORM_WIN32
