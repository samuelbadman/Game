#pragma once

#include "platform/graphics/abstract/graphics.h"

struct sQueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete() { return graphicsFamily.has_value() && computeFamily.has_value() && transferFamily.has_value(); }
};

class vulkanGraphics : public graphics
{
private:
#if defined(_DEBUG)
	vk::DebugUtilsMessengerEXT debugMessenger = {};
	vk::DispatchLoaderDynamic dldi = {};
#endif // defined(_DEBUG)

	uint32_t backBufferCount = 0;

	vk::Instance instance = {};

	vk::PhysicalDevice physicalDevice = {};
	sQueueFamilyIndices queueFamilyIndices = {};
	vk::Device device = {};

	vk::Queue graphicsQueue = {};
	vk::Queue computeQueue = {};
	vk::Queue transferQueue = {};

public:
	vulkanGraphics();
	~vulkanGraphics() final = default;

public:
	// Graphics interface
	void init(bool useWarp, uint32_t inBackBufferCount) final;
	void shutdown() final;
	void createSurface(void* hwnd, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<graphicsSurface>& outSurface) final;
	void destroySurface(std::shared_ptr<graphicsSurface>& surface) final;
	void resizeSurface(graphicsSurface* surface, uint32_t width, uint32_t height) final;
	void setSurfaceUseVSync(graphicsSurface* surface, const bool inUseVSync) final;
	void beginFrame() final;
	void render(const uint32_t numSurfaces, graphicsSurface* const* surfaces, const uint32_t renderDataCount, const sRenderData* const* renderData, const matrix4x4* const viewProjection) final;
	void endFrame(const uint32_t numSurfaces, graphicsSurface* const* surfaces) final;
	//void loadMesh(const size_t vertexCount, const sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, sMeshResources& outMeshResources) final;
	void loadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const sVertexPos3Norm3Col4UV2(* const vertices)[], const size_t* const indexCounts, const uint32_t(* const indices)[], sMeshResources** const outMeshResources) final;

private:
	void createInstanceLayersAndExtensionsConfiguration(std::vector<const char*>& outEnabledLayerNames, std::vector<const char*>& outEnabledExtensionNames);
	void createDeviceLayersAndExtensionsConfiguration(std::vector<const char*>& outEnabledLayerNames, std::vector<const char*>& outEnabledExtensionNames);

	void makeInstance();
	void destroyInstance();

	void makeDevice();
	void destroyDevice();


};