#pragma once

struct sQueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete() { return graphicsFamily.has_value() && computeFamily.has_value() && transferFamily.has_value(); }
};

class vulkanGraphics
{
private:
#if defined(_DEBUG)
	static constexpr bool breakOnDebugCallback = true;
	static constexpr VkDebugUtilsMessageSeverityFlagBitsEXT breakOnDebugCallbackMinSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	static constexpr VkDebugUtilsMessageSeverityFlagBitsEXT debugCallbackMinSeverityConsolePrint = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

	static vk::DebugUtilsMessengerEXT debugMessenger;
	static vk::DispatchLoaderDynamic dldi;
#endif // defined(_DEBUG)

	static vk::Instance instance;

	static vk::PhysicalDevice physicalDevice;
	static sQueueFamilyIndices queueFamilyIndices;
	static vk::Device device;

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

	static void makeInstance();
	static void destroyInstance();

	static void makeDevice();
	static void destroyDevice();
};