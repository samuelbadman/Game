#pragma once

extern wchar_t** platformGetArgcArgv(int32_t& outArgc);
extern void platformFreeArgv(wchar_t** argv);