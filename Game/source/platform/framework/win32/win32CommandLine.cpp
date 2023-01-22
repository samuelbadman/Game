#include "pch.h"

#if defined(PLATFORM_WIN32)

wchar_t** platformGetArgcArgv(int32_t& outArgc)
{
	return CommandLineToArgvW(GetCommandLineW(), &outArgc);
}

void platformFreeArgv(wchar_t** argv)
{
	LocalFree(argv);
}

#endif // PLATFORM_WIN32
