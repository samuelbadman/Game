#pragma once

class vulkanGraphics
{
private:
	static vk::Instance instance;
#if defined(_DEBUG)
	static vk::DebugUtilsMessengerEXT debugMessenger;
	static vk::DispatchLoaderDynamic dldi;
#endif // defined(DEBUG)

	static vk::PhysicalDevice physicalDevice;

public:
	static void init(bool useWarp, uint32_t inBackBufferCount);
	static void shutdown();
	static void createSurface(void* hwnd, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<class graphicsSurface>& outSurface);
	static void destroySurface(std::shared_ptr<class graphicsSurface>& surface);
	static void resizeSurface(class graphicsSurface* surface, uint32_t width, uint32_t height);
	static void setSurfaceUseVSync(class graphicsSurface* surface, const bool inUseVSync);
	static void beginFrame();
	static void render(const uint32_t numSurfaces, const class graphicsSurface* const* surfaces, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const class matrix4x4* const viewProjection);
	static void endFrame(const uint32_t numSurfaces, const class graphicsSurface* const* surfaces);
	//static void loadMesh(const size_t vertexCount, const struct sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, struct sMeshResources& outMeshResources);
	static void loadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const struct sVertexPos3Norm3Col4UV2(* const vertices)[], const size_t* const indexCounts, const uint32_t(* const indices)[], struct sMeshResources** const outMeshResources);

private:
	static void createInstanceLayersAndExtensionsConfiguration(std::vector<const char*>& outEnabledLayerNames, std::vector<const char*>& outEnabledExtensionNames);
	static void createDeviceLayersAndExtensionsConfiguration(std::vector<const char*>& outEnabledLayerNames, std::vector<const char*>& outEnabledExtensionNames);

#if defined(_DEBUG)
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif // defined(_DEBUG)

	static void choosePhysicalDevice(const vk::Instance& instance, const uint32_t enabledLayerCount, const char* const* enabledLayerNames,
		const uint32_t enabledExtensionCount, const char* const* enabledExtensionNames, vk::PhysicalDevice& outPhysicalDevice);

	static void makeInstance();
	static void destroyInstance();

	static void makeDevice();
};