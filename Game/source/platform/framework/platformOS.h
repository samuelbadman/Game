#pragma once

namespace platformLayer
{
	namespace os
	{
		extern void pollOS();
		extern void updateTiming(int64_t& outFps, double& outMs);
	}
}