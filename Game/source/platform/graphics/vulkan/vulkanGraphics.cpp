#include "pch.h"
#include "vulkanGraphics.h"
#include "platform/framework/platformMessageBox.h"
#include "stringHelper.h"
#include "platform/framework/platformConsole.h"

static void createVulkanInstance(vk::Instance& outInstance)
{
	// Enumerate vulkan api version
	uint32_t apiVersion;
	vkEnumerateInstanceVersion(&apiVersion);

	//platformConsolePrint(stringHelper::printf("system supported vulkan api version: %d.%d.%d.%d",
	//	VK_API_VERSION_VARIANT(apiVersion), VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion)));

	// Zero-out api patch version
	apiVersion &= ~(0xfffu);

	//platformConsolePrint(stringHelper::printf("creating instance with vulkan api version: %d.%d.%d.%d",
	//	VK_API_VERSION_VARIANT(apiVersion), VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion)));

	// Create application info
	vk::ApplicationInfo applicationInfo = vk::ApplicationInfo("vulkanGraphics", apiVersion, "vulkanEngine", apiVersion, apiVersion);

	// Create instance create info
	std::vector<const char*> enabledLayerNames;
	std::vector<const char*> enabledExtensionNames;
	vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo,
		static_cast<uint32_t>(enabledLayerNames.size()), enabledLayerNames.data(), static_cast<uint32_t>(enabledExtensionNames.size()), enabledExtensionNames.data(), nullptr);

	// Create instance
	outInstance = vk::createInstance(instanceCreateInfo);
}

vk::Instance vulkanGraphics::instance = {};

void vulkanGraphics::init(bool useWarp, uint32_t inBackBufferCount)
{
	createVulkanInstance(instance);
}

void vulkanGraphics::shutdown()
{
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
