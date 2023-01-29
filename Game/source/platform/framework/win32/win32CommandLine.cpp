#include "pch.h"

wchar_t** platformGetArgcArgv(int32_t& outArgc)
{
	return CommandLineToArgvW(GetCommandLineW(), &outArgc);
}

void platformFreeArgv(wchar_t** argv)
{
	LocalFree(argv);
}

