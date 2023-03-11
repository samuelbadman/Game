#include "pch.h"
#include "vulkanGraphics.h"
#include "platform/framework/abstract/platformMessageBox.h"
#include "sString.h"
#include "platform/framework/abstract/platformConsole.h"
#include "vulkanSurface.h"

#define VULKAN_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

#if defined(PLATFORM_WIN32)
#define VULKAN_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
#endif // defined(PLATFORM_WIN32)

static constexpr bool breakOnDebugCallback = true;
static constexpr VkDebugUtilsMessageSeverityFlagBitsEXT breakOnDebugCallbackMinSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
static constexpr VkDebugUtilsMessageSeverityFlagBitsEXT debugCallbackMinSeverityConsolePrint = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

struct swapchainSupportDetails
{
	vk::SurfaceCapabilitiesKHR capabilities = {};
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;

	bool isPresentModeSupported(const vk::PresentModeKHR& requestedPresentMode) const { return std::find(presentModes.begin(), presentModes.end(), requestedPresentMode) != presentModes.end(); }
	bool isFormatSupported(const vk::SurfaceFormatKHR& requestedSurfaceFormat) const { return std::find(formats.begin(), formats.end(), requestedSurfaceFormat) != formats.end(); }
};

static bool checkInstanceLayersAndExtensionsConfigurationSupported(const uint32_t enabledLayerCount, const char* const* enabledLayerNames,
	const uint32_t enabledExtensionCount, const char* const* enabledExtensionNames)
{
	// Get supported layers and extensions
	std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();
	platformLayer::console::consolePrint("supported instance extensions:");
	for (const vk::ExtensionProperties& extensionProperty : supportedExtensions)
	{
		platformLayer::console::consolePrint(sString::printf("    %s", extensionProperty.extensionName));
	}

	std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();
	platformLayer::console::consolePrint("supported instance layers:");
	for (const vk::LayerProperties& layerProperty : supportedLayers)
	{
		platformLayer::console::consolePrint(sString::printf("    %s", layerProperty.layerName));
	}

	// Check extensions
	for (uint32_t i = 0; i < enabledExtensionCount; ++i)
	{
		bool found = false;
		for (const vk::ExtensionProperties& extensionProperty : supportedExtensions)
		{
			if (strcmp(enabledExtensionNames[i], extensionProperty.extensionName) == 0)
			{
				found = true;
				platformLayer::console::consolePrint(sString::printf("requested %s instance extension is supported.", enabledExtensionNames[i]));
				break;
			}
		}

		if (!found)
		{
			platformLayer::console::consolePrint(sString::printf("requested %s instance extension is not supported.", enabledExtensionNames[i]));
			return false;
		}
	}

	// Check layers
	for (uint32_t i = 0; i < enabledLayerCount; ++i)
	{
		bool found = false;
		for (const vk::LayerProperties& layerProperty : supportedLayers)
		{
			if (strcmp(enabledLayerNames[i], layerProperty.layerName) == 0)
			{
				found = true;
				platformLayer::console::consolePrint(sString::printf("requested %s instance layer is supported.", enabledLayerNames[i]));
				break;
			}
		}

		if (!found)
		{
			platformLayer::console::consolePrint(sString::printf("requested %s instance layer is not supported.", enabledLayerNames[i]));
			return false;
		}
	}

	return true;
}

static bool checkDeviceLayersAndExtensionsConfigurationSupported(const vk::PhysicalDevice& device, const uint32_t enabledLayerCount, const char* const* enabledLayerNames,
	const uint32_t enabledExtensionCount, const char* const* enabledExtensionNames)
{
	// Get device properties
	vk::PhysicalDeviceProperties deviceProperties = device.getProperties();

	// Get supported layers and extensions
	std::vector<vk::ExtensionProperties> supportedExtensions = device.enumerateDeviceExtensionProperties();
	platformLayer::console::consolePrint(sString::printf("supported extensions by physical device (%s):", deviceProperties.deviceName));
	for (const vk::ExtensionProperties& extensionProperty : supportedExtensions)
	{
		platformLayer::console::consolePrint(sString::printf("    %s", extensionProperty.extensionName));
	}

	std::vector<vk::LayerProperties> supportedLayers = device.enumerateDeviceLayerProperties();
	platformLayer::console::consolePrint(sString::printf("supported layers by physical device (%s):", deviceProperties.deviceName));
	for (const vk::LayerProperties& layerProperty : supportedLayers)
	{
		platformLayer::console::consolePrint(sString::printf("    %s", layerProperty.layerName));
	}

	// Check extensions
	for (uint32_t i = 0; i < enabledExtensionCount; ++i)
	{
		bool found = false;
		for (const vk::ExtensionProperties& extensionProperty : supportedExtensions)
		{
			if (strcmp(enabledExtensionNames[i], extensionProperty.extensionName) == 0)
			{
				found = true;
				platformLayer::console::consolePrint(sString::printf("requested %s device extension is supported.", enabledExtensionNames[i]));
				break;
			}
		}

		if (!found)
		{
			platformLayer::console::consolePrint(sString::printf("requested %s device extension is not supported.", enabledExtensionNames[i]));
			return false;
		}
	}

	// Check layers
	for (uint32_t i = 0; i < enabledLayerCount; ++i)
	{
		bool found = false;
		for (const vk::LayerProperties& layerProperty : supportedLayers)
		{
			if (strcmp(enabledLayerNames[i], layerProperty.layerName) == 0)
			{
				found = true;
				platformLayer::console::consolePrint(sString::printf("requested %s device layer is supported.", enabledLayerNames[i]));
				break;
			}
		}

		if (!found)
		{
			platformLayer::console::consolePrint(sString::printf("requested %s device layer is not supported.", enabledLayerNames[i]));
			return false;
		}
	}

	return true;
}

static void createVulkanInstance(const uint32_t enabledLayerCount, const char* const* enabledLayerNames, const uint32_t enabledExtensionCount, const char* const* enabledExtensionNames,
	vk::Instance& outInstance)
{
	// Enumerate vulkan api version
	uint32_t apiVersion;
	vkEnumerateInstanceVersion(&apiVersion);

	platformLayer::console::consolePrint(sString::printf("system supported vulkan api version: %d.%d.%d.%d.",
		VK_API_VERSION_VARIANT(apiVersion), VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion)));

	// Zero-out api patch version
	apiVersion &= ~(0xfffu);

	platformLayer::console::consolePrint(sString::printf("creating vulkan instance with vulkan api version: %d.%d.%d.%d.",
		VK_API_VERSION_VARIANT(apiVersion), VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion)));

	// Create application info
	vk::ApplicationInfo applicationInfo = vk::ApplicationInfo("vulkanGraphics", apiVersion, "vulkanEngine", apiVersion, apiVersion);

	// Create instance create info
	if (!checkInstanceLayersAndExtensionsConfigurationSupported(enabledLayerCount, enabledLayerNames, enabledExtensionCount, enabledExtensionNames))
	{
		platformLayer::messageBox::showMessageBoxFatal("vulkanGraphics::createVulkanInstance: Layer extension configuration is not supported.");
	}

	vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo,
		enabledLayerCount, enabledLayerNames, enabledExtensionCount, enabledExtensionNames, nullptr);

	// Create instance
	try
	{
		outInstance = vk::createInstance(instanceCreateInfo);
	}
	catch (vk::SystemError err)
	{
		platformLayer::messageBox::showMessageBoxFatal("vulkanGraphics::createVulkanInstance: Failed to create vulkan instance.");
	}
}

#if defined(_DEBUG)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= debugCallbackMinSeverityConsolePrint)
	{
		platformLayer::console::consolePrint(sString::printf("vulkan debug callback: %s", pCallbackData->pMessage));
	}

#if defined(PLATFORM_WIN32)
	if constexpr (breakOnDebugCallback)
	{
		if (messageSeverity >= breakOnDebugCallbackMinSeverity)
		{
			DebugBreak();
		}
	}
#endif // defined(PLATFORM_WIN32)

	// Note: Debug callback has likely been triggered. See message in console window
	return VK_FALSE;
}
#endif // defined(_DEBUG)

static void makeDebugMessenger(const vk::Instance& instance, const vk::DispatchLoaderDynamic& dldi, PFN_vkDebugUtilsMessengerCallbackEXT userCallback,
	vk::DebugUtilsMessengerEXT& outDebugMessenger)
{
	vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(vk::DebugUtilsMessengerCreateFlagBitsEXT(),
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | 
		vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding,
		userCallback,
		nullptr);

	try
	{
		outDebugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
	}
	catch (vk::SystemError err)
	{
		platformLayer::messageBox::showMessageBoxFatal("vulkanGraphics::makeDebugMessenger: Failed to create debug messenger.");
	}
}

static void getPhysicalDevice(const vk::Instance& instance, const uint32_t enabledLayerCount, const char* const* enabledLayerNames,
	const uint32_t enabledExtensionCount, const char* const* enabledExtensionNames, vk::PhysicalDevice& outPhysicalDevice)
{
	// Get available physical devices
	std::vector<vk::PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();

	// Todo: Find best suitable physical device from available devices
	// Get first available physical device that supports the requested layers and extensions
	for (const vk::PhysicalDevice& device : availableDevices)
	{
		vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = device.getMemoryProperties();
		vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

		if (checkDeviceLayersAndExtensionsConfigurationSupported(device, enabledLayerCount, enabledLayerNames, enabledExtensionCount, enabledExtensionNames) &&
			deviceMemoryProperties.memoryHeapCount != 0)
		{
			outPhysicalDevice = device;
			platformLayer::console::consolePrint(sString::printf("selected physical device: (%s)", deviceProperties.deviceName));
			return;
		}
	}

	platformLayer::messageBox::showMessageBoxFatal("vulkanGraphics::getPhysicalDevice: could not find a suitable physical device.");
}

static void findQueueFamilies(const vk::PhysicalDevice& physicalDevice, sQueueFamilyIndices& outQueueFamilyIndices)
{
	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

	vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
	platformLayer::console::consolePrint(sString::printf("physical device (%s) supports %d queue families.", deviceProperties.deviceName, queueFamilies.size()));

	uint32_t i = 0;
	for (const vk::QueueFamilyProperties& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			outQueueFamilyIndices.graphicsFamily = i;
			platformLayer::console::consolePrint(sString::printf("index %d set for graphics queue family.", i));
		}
		else if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
		{
			outQueueFamilyIndices.computeFamily = i;
			platformLayer::console::consolePrint(sString::printf("index %d set for compute queue family.", i));
		}
		else if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
		{
			outQueueFamilyIndices.transferFamily = i;
			platformLayer::console::consolePrint(sString::printf("index %d set for transfer queue family.", i));
		}

		if (outQueueFamilyIndices.isComplete())
		{
			break;
		}

		++i;
	}

	if (!outQueueFamilyIndices.isComplete())
	{
		platformLayer::messageBox::showMessageBoxFatal(sString::printf("vulkanGraphics::findQueueFamilies: Physical device (%s) does not support all required queue families."));
	}
}

static void createLogicalDevice(const vk::PhysicalDevice& physicalDevice, 
	const sQueueFamilyIndices& queueFamilyIndices, 
	const uint32_t graphicsQueueCount,
	const float graphicsQueuePriority,
	const uint32_t computeQueueCount,
	const float computeQueuePriority,
	const uint32_t transferQueueCount,
	const float transferQueuePriority,
	const std::vector<const char*>& enabledLayerNames, 
	const std::vector<const char*>& enabledExtensionNames,
	vk::Device& outDevice
)
{
	// Get physical device features
	vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.getFeatures();

	// Create device queue create info for each queue family
	std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
	deviceQueueCreateInfos.resize(3);
	deviceQueueCreateInfos[0] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamilyIndices.graphicsFamily.value(), graphicsQueueCount, &graphicsQueuePriority);
	deviceQueueCreateInfos[1] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamilyIndices.computeFamily.value(), computeQueueCount, &computeQueuePriority);
	deviceQueueCreateInfos[2] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamilyIndices.transferFamily.value(), transferQueueCount, &transferQueuePriority);

	// Create device create info
	vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueCreateInfos, enabledLayerNames, enabledExtensionNames, &deviceFeatures);

	// Create logical device
	try 
	{
		outDevice = physicalDevice.createDevice(deviceCreateInfo);
	}
	catch (vk::SystemError err)
	{
		platformLayer::messageBox::showMessageBoxFatal("vulkanGraphics::createLogicalDevice: Failed to create device.");
	}
}

static void getQueue(const vk::Device& device, const uint32_t queueFamilyIndex, const uint32_t queueIndex, vk::Queue& outQueue)
{
	try
	{
		outQueue = device.getQueue(queueFamilyIndex, queueIndex);
	}
	catch (vk::SystemError err)
	{
		platformLayer::messageBox::showMessageBoxFatal(sString::printf("vulkanGraphics::getQueue: Failed to get queue with index %d from queue family index %d.", queueIndex, queueFamilyIndex));
	}
}

static swapchainSupportDetails getSwapchainSupportDetails(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
{
	swapchainSupportDetails out = {};
	out.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	out.formats = physicalDevice.getSurfaceFormatsKHR(surface);
	out.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
	return out;
}

static void createSwapchain(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, const uint32_t imageCount, const vk::PresentModeKHR& presentMode, const vk::SurfaceKHR& surface, const vk::SurfaceFormatKHR& surfaceFormat, const vk::Extent2D& extent, vk::SwapchainKHR& outSwapchain)
{
	// Describe swap chain create info
	vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
		vk::SwapchainCreateFlagsKHR(),
		surface, imageCount,
		surfaceFormat.format, surfaceFormat.colorSpace,
		extent, 1,
		vk::ImageUsageFlagBits::eColorAttachment
	);

	createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	createInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

	// Create the swap chain
	try
	{
		outSwapchain = device.createSwapchainKHR(createInfo);
	}
	catch (vk::SystemError err)
	{
		platformLayer::messageBox::showMessageBoxFatal("vulkanGraphics::createSwapchain: Failed to create swap chain.");
	}
}

vulkanGraphics::vulkanGraphics()
	: graphics(eGraphicsApi::vulkan)
{
}

void vulkanGraphics::init(bool useWarp, uint32_t inBackBufferCount)
{
	backBufferCount = inBackBufferCount;
	makeInstance();
	makeDevice();
}

void vulkanGraphics::shutdown()
{
	destroyDevice();
	destroyInstance();
}

void vulkanGraphics::createSurface(void* hwnd, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<graphicsSurface>& outSurface)
{
	outSurface = std::make_shared<vulkanSurface>();
	vulkanSurface* apiSurface = outSurface->as<vulkanSurface>();

	// Create surface for platform
#if defined(PLATFORM_WIN32)
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = {};
	win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfo.hwnd = static_cast<HWND>(hwnd);
	win32SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

	VkSurfaceKHR cSurface;
	if(vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfo, nullptr, &cSurface) != VK_SUCCESS)
	{
		platformLayer::messageBox::showMessageBoxFatal("vulkanGraphics::createSurface: Failed to create win32 surface.");
	}

	apiSurface->surface = cSurface;
#endif // defined(PLATFORM_WIN32)

	// Get swap chain support details
	swapchainSupportDetails supportDetails = getSwapchainSupportDetails(physicalDevice, apiSurface->surface);

	// Get swap chain present mode
	vk::PresentModeKHR swapchainPresentMode = vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
	if (!(supportDetails.isPresentModeSupported(swapchainPresentMode)))
	{
		platformLayer::messageBox::showMessageBox(platformLayer::messageBox::eMessageLevel::warning, sString::printf( "vulkanGraphics::createSurface: present mode (%s) is not supported by the physical device. Falling back on Fifo present mode.", vk::to_string(swapchainPresentMode)));

		swapchainPresentMode = vk::PresentModeKHR::eFifo;
	}

	// Get swap chain format
	vk::SurfaceFormatKHR swapchainFormat = vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear);
	if (!(supportDetails.isFormatSupported(swapchainFormat)))
	{
		platformLayer::messageBox::showMessageBox(platformLayer::messageBox::eMessageLevel::warning, sString::printf(
			"vulkanGraphics::createSurface: surface format (%s, %s) is not supported by the physical device. Falling back on first available format (%s, %s)", 
			vk::to_string(swapchainFormat.format), vk::to_string(swapchainFormat.colorSpace), vk::to_string(supportDetails.formats[0].format), vk::to_string(supportDetails.formats[0].colorSpace)));

		swapchainFormat = supportDetails.formats[0];
	}

	// Get swap chain extent
	vk::Extent2D swapchainExtent = vk::Extent2D(width, height);

	// Get swap chain image count
	uint32_t swapchainImageCount = backBufferCount;

	// Check if the surface supports the requested amount of images
	if (swapchainImageCount < supportDetails.capabilities.minImageCount + 1)
	{
		swapchainImageCount = supportDetails.capabilities.minImageCount + 1;
	}

	// Check if the max image count needs to be checked as a max image count of 0 can be an unlimited amount of swapchain images
	if (supportDetails.capabilities.maxImageCount > 0)
	{
		if (swapchainImageCount > supportDetails.capabilities.maxImageCount)
		{
			swapchainImageCount = supportDetails.capabilities.maxImageCount;
		}
	}

	// Create swap chain
	createSwapchain(device, physicalDevice, backBufferCount, swapchainPresentMode, apiSurface->surface, swapchainFormat, swapchainExtent, apiSurface->swapchain);

	// Get swap chain images and store format and extent in the surface
	apiSurface->images.reserve(static_cast<size_t>(swapchainImageCount));
	apiSurface->images = device.getSwapchainImagesKHR(apiSurface->swapchain);

	// Create image views for swap chain images
	apiSurface->imageViews.reserve(static_cast<size_t>(swapchainImageCount));
	for (const vk::Image& image : apiSurface->images)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo(vk::ImageViewCreateFlags(),
			image, vk::ImageViewType::e2D, swapchainFormat.format, vk::ComponentMapping(),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1), nullptr);

		try
		{
			apiSurface->imageViews.emplace_back(device.createImageView(imageViewCreateInfo));
		}
		catch (vk::SystemError err)
		{
			platformLayer::messageBox::showMessageBoxFatal("vulkanGraphics::createSurface: Failed to create swap chain image view.");
		}
	}
}

void vulkanGraphics::destroySurface(std::shared_ptr<graphicsSurface>& surface)
{
	assert(surface->getApi() == eGraphicsApi::vulkan);

	vulkanSurface* apiSurface = surface->as<vulkanSurface>();

	// Destroy image views
	for (const vk::ImageView& imageView : apiSurface->imageViews)
	{
		device.destroyImageView(imageView);
	}
	apiSurface->imageViews.clear();

	// Destroy swap chain and images
	device.destroySwapchainKHR(apiSurface->swapchain);
	apiSurface->images.clear();

	// Destroy vulkan surface
	instance.destroySurfaceKHR(apiSurface->surface);

	// Reset graphics surface
	surface.reset();
}

void vulkanGraphics::resizeSurface(graphicsSurface* surface, uint32_t width, uint32_t height)
{
}

void vulkanGraphics::setSurfaceUseVSync(graphicsSurface* surface, const bool inUseVSync)
{
	platformLayer::messageBox::showMessageBox(platformLayer::messageBox::eMessageLevel::message, "Todo: Surface must be destroyed and recreated to toggle vsync using vulkan api.");
}

void vulkanGraphics::beginFrame()
{
}

void vulkanGraphics::render(const uint32_t numSurfaces, graphicsSurface* const* surfaces, const uint32_t renderDataCount, const sRenderData* const* renderData, 
	const matrix4x4* const viewProjection)
{
}

void vulkanGraphics::endFrame(const uint32_t numSurfaces, graphicsSurface* const* surfaces)
{
}

//void vulkanGraphics::loadMesh(const size_t vertexCount, const sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, sMeshResources& outMeshResources)
//{
//}

void vulkanGraphics::loadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const sVertexPos3Norm3Col4UV2(* const vertices)[], 
	const size_t* const indexCounts, const uint32_t(* const indices)[], sMeshResources** const outMeshResources)
{
}

void vulkanGraphics::createInstanceLayersAndExtensionsConfiguration(std::vector<const char*>& outEnabledLayerNames, std::vector<const char*>& outEnabledExtensionNames)
{
#if defined(_DEBUG)
	outEnabledLayerNames.emplace_back(VULKAN_VALIDATION_LAYER_NAME);
	outEnabledExtensionNames.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // defined(_DEBUG)

	outEnabledExtensionNames.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(PLATFORM_WIN32)
	outEnabledExtensionNames.emplace_back(VULKAN_WIN32_SURFACE_EXTENSION_NAME);
#endif // defined(PLATFORM_WIN32)
}

void vulkanGraphics::createDeviceLayersAndExtensionsConfiguration(std::vector<const char*>& outEnabledLayerNames, std::vector<const char*>& outEnabledExtensionNames)
{
#if defined(_DEBUG)
	outEnabledExtensionNames.reserve(1);
	outEnabledLayerNames.emplace_back(VULKAN_VALIDATION_LAYER_NAME);
#endif // defined(_DEBUG)

	outEnabledExtensionNames.reserve(1);
	outEnabledExtensionNames.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void vulkanGraphics::makeInstance()
{
	std::vector<const char*> enabledLayerNames;
	std::vector<const char*> enabledExtensionNames;
	createInstanceLayersAndExtensionsConfiguration(enabledLayerNames, enabledExtensionNames);
	createVulkanInstance(static_cast<uint32_t>(enabledLayerNames.size()), enabledLayerNames.data(), 
		static_cast<uint32_t>(enabledExtensionNames.size()), enabledExtensionNames.data(), instance);

#if defined(_DEBUG)
	dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
	makeDebugMessenger(instance, dldi, debugCallback, debugMessenger);
#endif // defined(_DEBUG)
}

void vulkanGraphics::destroyInstance()
{
#if defined(_DEBUG)
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
#endif // defined(_DEBUG)

	instance.destroy();
}

void vulkanGraphics::makeDevice()
{
	// Create device layers and extensions configuration
	std::vector<const char*> enabledLayerNames;
	std::vector<const char*> enabledExtensionNames;
	createDeviceLayersAndExtensionsConfiguration(enabledLayerNames, enabledExtensionNames);

	// Get physical device
	getPhysicalDevice(instance, static_cast<uint32_t>(enabledLayerNames.size()), enabledLayerNames.data(),
		static_cast<uint32_t>(enabledExtensionNames.size()), enabledExtensionNames.data(), physicalDevice);

	// Find queue family indices for physical device
	findQueueFamilies(physicalDevice, queueFamilyIndices);

	// Create logical device
	createLogicalDevice(physicalDevice, queueFamilyIndices, 1, 1.0f, 1, 1.0f, 1, 1.0f, enabledLayerNames, enabledExtensionNames, device);

	// Get queues
	getQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, graphicsQueue);
	getQueue(device, queueFamilyIndices.computeFamily.value(), 0, computeQueue);
	getQueue(device, queueFamilyIndices.transferFamily.value(), 0, transferQueue);
}

void vulkanGraphics::destroyDevice()
{
	device.destroy();
}

