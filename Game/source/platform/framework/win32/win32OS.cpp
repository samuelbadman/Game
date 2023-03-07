#include "pch.h"

namespace platformLayer
{
	namespace os
	{
		void pollOS()
		{
			// Dispatch windows messages
			MSG msg = {};
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}
