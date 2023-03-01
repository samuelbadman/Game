#pragma once

namespace platformLayer
{
	extern wchar_t** getArgcArgv(int32_t& outArgc);
	extern void freeArgv(wchar_t** argv);
}