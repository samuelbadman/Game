#include "pch.h"
#include "graphics.h"

#if defined(PLATFORM_WIN32)
#include "direct3D12/direct3D12Graphics.h"
#endif // defined(PLATFORM_WIN32)

#include "vulkan/vulkanGraphics.h"

void graphics::create(const eGraphicsApi graphicsApi, std::shared_ptr<graphics>& outGraphics)
{
	switch (graphicsApi)
	{
	case eGraphicsApi::direct3d12:
	{
		outGraphics = std::make_shared<direct3d12Graphics>();
	}
	break;

	case eGraphicsApi::vulkan:
	{
		outGraphics = std::make_shared<vulkanGraphics>();
	}
	break;
	}
}
