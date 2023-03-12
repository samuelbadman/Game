#include "pch.h"
#include "graphics.h"

#if defined(PLATFORM_WIN32)
#include "platform/graphics/direct3D12/direct3D12Graphics.h"
#endif // defined(PLATFORM_WIN32)

#include "platform/graphics/vulkan/vulkanGraphics.h"

void graphics::create(const eGraphicsApi graphicsApi, std::shared_ptr<graphics>& outGraphics)
{
	switch (graphicsApi)
	{
#if defined(PLATFORM_WIN32)
		// Implement case for supported graphics api's on this platform
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
#endif // defined(PLATFORM_WIN32)

	// Unhandled(unsupported api) cases will fallback on the default case
	default:
	{
		outGraphics.reset();
	}
	break;
	}
}

graphics::graphics(const eGraphicsApi inApi)
	: graphicsObject(inApi)
{
}
