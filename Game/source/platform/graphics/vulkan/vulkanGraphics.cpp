#include "pch.h"
#include "vulkanGraphics.h"
#include "platform/framework/platformMessageBox.h"
#include "stringHelper.h"
#include "platform/framework/platformConsole.h"

#define VULKAN_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

#if defined(PLATFORM_WIN32)
#define VULKAN_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
#endif // defined(PLATFORM_WIN32)

static bool checkInstanceLayersAndExtensionsConfigurationSupported(const uint32_t enabledLayerCount, const char* const* enabledLayerNames,
	const uint32_t enabledExtensionCount, const char* const* enabledExtensionNames)
{
	// Get supported layers and extensions
	std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();
	platformConsolePrint("supported instance extensions:");
	for (const vk::ExtensionProperties& extensionProperty : supportedExtensions)
	{
		platformConsolePrint(stringHelper::printf("    %s", extensionProperty.extensionName));
	}

	std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();
	platformConsolePrint("supported instance layers:");
	for (const vk::LayerProperties& layerProperty : supportedLayers)
	{
		platformConsolePrint(stringHelper::printf("    %s", layerProperty.layerName));
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
				platformConsolePrint(stringHelper::printf("requested %s instance extension is supported", enabledExtensionNames[i]));
				break;
			}
		}

		if (!found)
		{
			platformConsolePrint(stringHelper::printf("requested %s instance extension is not supported", enabledExtensionNames[i]));
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
				platformConsolePrint(stringHelper::printf("requested %s instance layer is supported", enabledLayerNames[i]));
				break;
			}
		}

		if (!found)
		{
			platformConsolePrint(stringHelper::printf("requested %s instance layer is not supported", enabledLayerNames[i]));
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
	platformConsolePrint(stringHelper::printf("supported extensions by physical device (%s):", deviceProperties.deviceName));
	for (const vk::ExtensionProperties& extensionProperty : supportedExtensions)
	{
		platformConsolePrint(stringHelper::printf("    %s", extensionProperty.extensionName));
	}

	std::vector<vk::LayerProperties> supportedLayers = device.enumerateDeviceLayerProperties();
	platformConsolePrint(stringHelper::printf("supported layers by physical device (%s):", deviceProperties.deviceName));
	for (const vk::LayerProperties& layerProperty : supportedLayers)
	{
		platformConsolePrint(stringHelper::printf("    %s", layerProperty.layerName));
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
				platformConsolePrint(stringHelper::printf("requested %s device extension is supported", enabledExtensionNames[i]));
				break;
			}
		}

		if (!found)
		{
			platformConsolePrint(stringHelper::printf("requested %s device extension is not supported", enabledExtensionNames[i]));
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
				platformConsolePrint(stringHelper::printf("requested %s device layer is supported", enabledLayerNames[i]));
				break;
			}
		}

		if (!found)
		{
			platformConsolePrint(stringHelper::printf("requested %s device layer is not supported", enabledLayerNames[i]));
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

	platformConsolePrint(stringHelper::printf("system supported vulkan api version: %d.%d.%d.%d",
		VK_API_VERSION_VARIANT(apiVersion), VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion)));

	// Zero-out api patch version
	apiVersion &= ~(0xfffu);

	platformConsolePrint(stringHelper::printf("creating vulkan instance with vulkan api version: %d.%d.%d.%d",
		VK_API_VERSION_VARIANT(apiVersion), VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion)));

	// Create application info
	vk::ApplicationInfo applicationInfo = vk::ApplicationInfo("vulkanGraphics", apiVersion, "vulkanEngine", apiVersion, apiVersion);

	// Create instance create info
	if (!checkInstanceLayersAndExtensionsConfigurationSupported(enabledLayerCount, enabledLayerNames, enabledExtensionCount, enabledExtensionNames))
	{
		platformMessageBoxFatal("vulkanGraphics::createVulkanInstance: layer extension configuration is not supported");
	}

	vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo,
		enabledLayerCount, enabledLayerNames, enabledExtensionCount, enabledExtensionNames, nullptr);

	// Create instance
	outInstance = vk::createInstance(instanceCreateInfo);
}

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

	outDebugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
}

#if defined(_DEBUG)
static constexpr bool breakOnDebugCallback = true;
#endif // defined(_DEBUG)

vk::Instance vulkanGraphics::instance = {};
#if defined(_DEBUG)
vk::DebugUtilsMessengerEXT vulkanGraphics::debugMessenger = nullptr;
vk::DispatchLoaderDynamic vulkanGraphics::dldi;
#endif // defined(_DEBUG)

vk::PhysicalDevice vulkanGraphics::physicalDevice = nullptr;

void vulkanGraphics::init(bool useWarp, uint32_t inBackBufferCount)
{
	makeInstance();
	makeDevice();
}

void vulkanGraphics::shutdown()
{
	destroyInstance();
}

void vulkanGraphics::createSurface(void* hwnd, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<class graphicsSurface>& outSurface)
{
}

void vulkanGraphics::destroySurface(std::shared_ptr<class graphicsSurface>& surface)
{
}

void vulkanGraphics::resizeSurface(graphicsSurface* surface, uint32_t width, uint32_t height)
{
}

void vulkanGraphics::setSurfaceUseVSync(graphicsSurface* surface, const bool inUseVSync)
{
}

void vulkanGraphics::beginFrame()
{
}

void vulkanGraphics::render(const uint32_t numSurfaces, const graphicsSurface* const* surfaces, const uint32_t renderDataCount, const sRenderData* const* renderData, 
	const matrix4x4* const viewProjection)
{
}

void vulkanGraphics::endFrame(const uint32_t numSurfaces, const graphicsSurface* const* surfaces)
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

#if defined(_DEBUG)
VKAPI_ATTR VkBool32 VKAPI_CALL vulkanGraphics::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	platformConsolePrint(stringHelper::printf("vulkan debug callback: %s", pCallbackData->pMessage));

#if defined(PLATFORM_WIN32)
	if constexpr (breakOnDebugCallback)
	{
		// Note: Debug callback has been triggered. See message in console window
		DebugBreak();
	}
#endif // defined(PLATFORM_WIN32)

	return VK_FALSE;
}
#endif // defined(_DEBUG)

void vulkanGraphics::getPhysicalDevice(const vk::Instance& instance, const uint32_t enabledLayerCount, const char* const* enabledLayerNames,
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
			platformConsolePrint(stringHelper::printf("selected physical device: (%s)", deviceProperties.deviceName));
			return;
		}
	}

	platformMessageBoxFatal("vulkanGraphics::getPhysicalDevice: could not find a suitable physical device.");
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
	std::vector<const char*> enabledLayerNames;
	std::vector<const char*> enabledExtensionNames;
	createDeviceLayersAndExtensionsConfiguration(enabledLayerNames, enabledExtensionNames);

	getPhysicalDevice(instance, static_cast<uint32_t>(enabledLayerNames.size()), enabledLayerNames.data(),
		static_cast<uint32_t>(enabledExtensionNames.size()), enabledExtensionNames.data(), physicalDevice);

}
