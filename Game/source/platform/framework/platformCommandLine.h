#pragma once

extern wchar_t** platformGetCommandLineArguments(int32_t& outNumArgs);
extern void platformFreeCommandLineArguments(void* args);