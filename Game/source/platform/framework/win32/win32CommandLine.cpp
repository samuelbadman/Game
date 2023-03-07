#include "pch.h"

namespace platformLayer
{
	namespace commandLine
	{
		wchar_t** getArgcArgv(int32_t& outArgc)
		{
			return CommandLineToArgvW(GetCommandLineW(), &outArgc);
		}

		void freeArgv(wchar_t** argv)
		{
			LocalFree(argv);
		}
	}
}
