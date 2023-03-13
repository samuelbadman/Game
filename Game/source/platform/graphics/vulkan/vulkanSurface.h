#pragma once

#include "platform/graphics/abstract/graphicsSurface.h"

class vulkanSurface : public graphicsSurface
{
public:
	vk::SurfaceKHR surface = {};
	vk::SwapchainKHR swapchain = {};
	std::vector<vk::Image> images = {};
	std::vector<vk::ImageView> imageViews = {};
	vk::Image depthStencilImage = {};

public:
	vulkanSurface();
	~vulkanSurface() final = default;
};