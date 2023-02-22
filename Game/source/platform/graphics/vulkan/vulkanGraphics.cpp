#include "pch.h"
#include "vulkanGraphics.h"
#include "platform/framework/platformMessageBox.h"
#include "stringHelper.h"
#include "platform/framework/platformConsole.h"

static bool checkLayersAndExtensionsConfigurationSupported(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	// Get supported layers and extensions
	std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();
	platformConsolePrint("supported instance extensions:");
	for (const vk::ExtensionProperties& extensionProperty : supportedExtensions)
	{
		platformConsolePrint(extensionProperty.extensionName);
	}
	platformConsolePrint("===============================");

	std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();
	platformConsolePrint("supported instance layers:");
	for (const vk::LayerProperties& layerProperty : supportedLayers)
	{
		platformConsolePrint(layerProperty.layerName);
	}
	platformConsolePrint("===============================");

	// Check extensions
	for (const char* extension : extensions)
	{
		bool found = false;
		for (const vk::ExtensionProperties& extensionProperty : supportedExtensions)
		{
			if (strcmp(extension, extensionProperty.extensionName) == 0)
			{
				found = true;
				platformConsolePrint(stringHelper::printf("requested %s instance extension enabled", extension));
				break;
			}
		}

		if (!found)
		{
			platformConsolePrint(stringHelper::printf("requested %s instance extension is not supported", extension));
			return false;
		}
	}

	// Check layers
	for (const char* layer : layers)
	{
		bool found = false;
		for (const vk::LayerProperties& layerProperty : supportedLayers)
		{
			if (strcmp(layer, layerProperty.layerName) == 0)
			{
				found = true;
				platformConsolePrint(stringHelper::printf("requested %s instance layer enabled", layer));
				break;
			}
		}

		if (!found)
		{
			platformConsolePrint(stringHelper::printf("requested %s instance layer is not supported", layer));
			return false;
		}
	}

	return true;
}

static void createInstanceLayersAndExtensionsConfiguration(std::vector<const char*>& outEnabledLayerNames, std::vector<const char*>& outEnabledExtensionNames)
{
#if defined(_DEBUG)
	outEnabledLayerNames.emplace_back("VK_LAYER_KHRONOS_validation");

	outEnabledExtensionNames.emplace_back("VK_EXT_debug_utils");
#endif // defined(_DEBUG)

	outEnabledExtensionNames.emplace_back("VK_KHR_surface");
#if defined(PLATFORM_WIN32)
	outEnabledExtensionNames.emplace_back("VK_KHR_win32_surface");
#endif // defined(PLATFORM_WIN32)
}

static void createVulkanInstance(vk::Instance& outInstance)
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

	// Create layers and extensions configuration and check it is supported by the system
	std::vector<const char*> enabledLayerNames;
	std::vector<const char*> enabledExtensionNames;
	createInstanceLayersAndExtensionsConfiguration(enabledLayerNames, enabledExtensionNames);

	if (!checkLayersAndExtensionsConfigurationSupported(enabledLayerNames, enabledExtensionNames))
	{
		platformMessageBoxFatal("vulkanGraphics::createVulkanInstance: layer extension configuration is not supported");
	}

	// Create instance create info
	vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo,
		static_cast<uint32_t>(enabledLayerNames.size()), enabledLayerNames.data(), static_cast<uint32_t>(enabledExtensionNames.size()), enabledExtensionNames.data(), nullptr);

	// Create instance
	outInstance = vk::createInstance(instanceCreateInfo);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	platformConsolePrint(stringHelper::printf("vulkan validation layer: %s", pCallbackData->pMessage));
	return VK_FALSE;
}

static void makeDebugMessenger(const vk::Instance& instance, const vk::DispatchLoaderDynamic& dldi, vk::DebugUtilsMessengerEXT& outDebugMessenger)
{
	vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(vk::DebugUtilsMessengerCreateFlagBitsEXT(),
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding,
		debugCallback,
		nullptr);

	outDebugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
}

vk::Instance vulkanGraphics::instance = {};
vk::DebugUtilsMessengerEXT vulkanGraphics::debugMessenger = nullptr;
vk::DispatchLoaderDynamic vulkanGraphics::dldi;

void vulkanGraphics::init(bool useWarp, uint32_t inBackBufferCount)
{
	createVulkanInstance(instance);

#if defined(_DEBUG)
	dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
	makeDebugMessenger(instance, dldi, debugMessenger);
#endif // defined(_DEBUG)


}

void vulkanGraphics::shutdown()
{
#if defined(_DEBUG)
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
#endif // defined(_DEBUG)

	instance.destroy();
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

void vulkanGraphics::render(const uint32_t numSurfaces, const graphicsSurface* const* surfaces, const uint32_t renderDataCount, const sRenderData* const* renderData, const matrix4x4* const viewProjection)
{
}

void vulkanGraphics::endFrame(const uint32_t numSurfaces, const graphicsSurface* const* surfaces)
{
}

//void vulkanGraphics::loadMesh(const size_t vertexCount, const sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, sMeshResources& outMeshResources)
//{
//}

void vulkanGraphics::loadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const sVertexPos3Norm3Col4UV2(* const vertices)[], const size_t* const indexCounts, const uint32_t(* const indices)[], sMeshResources** const outMeshResources)
{
}
