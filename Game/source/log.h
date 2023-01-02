#pragma once

#include "platform/win32/win32Console.h"

#define LOGGING_ENABLED 1

#if LOGGING_ENABLED
#define LOG_INIT()	const bool consoleInitResult = win32Console::init();\
if (!consoleInitResult)\
{\
	MessageBoxA(0, "Failed to init console.", "Error", MB_OK | MB_ICONERROR);\
	return -1;\
}
#define LOG_SHUTDOWN() const bool consoleShutdownResult = win32Console::shutdown();\
if (!consoleShutdownResult)\
{\
	MessageBoxA(0, "Failed to shutdown console.", "Error", MB_OK | MB_ICONERROR);\
	return -1;\
}
#define LOG(message) win32Console::print(message);
#else
#define LOG_INIT()
#define LOG_SHUTDOWN()
#define LOG(message)
#endif // LOGGING_ENABLED