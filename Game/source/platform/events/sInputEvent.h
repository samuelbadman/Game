#pragma once

namespace platformLayer
{
	namespace window
	{
		class platformWindow;
	}

	namespace input
	{
		struct sInputEvent
		{
			// The platform window that generated the input event if the input came from a window
			platformLayer::window::platformWindow* window = nullptr;

			bool repeatedKey = false;
			int16_t input = 0;
			int8_t port = 0;
			float data = 0.f;
		};
	}
}