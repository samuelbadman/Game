#include "pch.h"

#if defined(PLATFORM_WIN32)

#include "platform/framework/platformCommandLine.h"

wchar_t** platformGetCommandLineArguments(int32_t& outNumArgs)
{
	return CommandLineToArgvW(GetCommandLineW(), &outNumArgs);
}

void platformFreeCommandLineArguments(void* args)
{
	LocalFree(args);
}

#endif // PLATFORM_WIN32
