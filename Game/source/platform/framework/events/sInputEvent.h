#pragma once

namespace platformLayer
{
	namespace input
	{
		struct sInputEvent
		{
			bool repeatedKey = false;
			int16_t input = 0;
			int8_t port = 0;
			float data = 0.f;
		};
	}
}