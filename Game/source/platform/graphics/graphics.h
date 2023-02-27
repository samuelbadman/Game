#pragma once

#include "graphicsApi.h"

class graphics
{
public:
	static void create(const eGraphicsApi graphicsApi, std::shared_ptr<graphics>& outGraphics);

public:
	virtual ~graphics() = default;

public:
	virtual void init(const bool softwareRenderer, const uint32_t backBufferCount) = 0;
	virtual void shutdown() = 0;
	virtual void createSurface(void* platformWindowHandle, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<class graphicsSurface>& outSurface) = 0;
	virtual void destroySurface(std::shared_ptr<class graphicsSurface>& surface) = 0;
	virtual void resizeSurface(class graphicsSurface* surface, uint32_t width, uint32_t height) = 0;
	virtual void setSurfaceUseVSync(class graphicsSurface* surface, const bool inUseVSync) = 0;
	virtual void beginFrame() = 0;
	virtual void render(const uint32_t numSurfaces, const class graphicsSurface* const* surfaces, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const class matrix4x4* const viewProjection) = 0;
	virtual void endFrame(const uint32_t numRenderedSurfaces, const class graphicsSurface* const* renderedSurfaces) = 0;
	//virtual void loadMesh(const size_t vertexCount, const struct sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, struct sMeshResources& outMeshResources) = 0;
	virtual void loadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const struct sVertexPos3Norm3Col4UV2(* const vertices)[], const size_t* const indexCounts, const uint32_t(* const indices)[], struct sMeshResources** const outMeshResources) = 0;
};
