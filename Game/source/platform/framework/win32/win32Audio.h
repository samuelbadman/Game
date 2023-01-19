#pragma once

#if defined(PLATFORM_WIN32)

#include "platform/framework/PlatformAudio.h"

class win32Audio
{
public:
	static void init();
	static void destroyMasterVoice();
};

#endif // PLATFORM_WIN32
