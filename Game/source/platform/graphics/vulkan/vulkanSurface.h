#pragma once

#include "platform/graphics/graphicsSurface.h"

class vulkanSurface : public graphicsSurface
{
public:
	vk::SurfaceKHR surface = {};
};