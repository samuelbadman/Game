#pragma once

#include "graphicsObject.h"

class graphicsSurface;
class matrix4x4;

struct sVertexPos3Norm3Col4UV2;
struct sRenderData; 
struct sMeshResources;

class graphics : public graphicsObject
{
public:
	static void create(const eGraphicsApi graphicsApi, std::shared_ptr<graphics>& outGraphics);

public:
	graphics(const eGraphicsApi inApi);
	~graphics() override = default;

public:
	virtual void init(const bool softwareRenderer, const uint32_t backBufferCount) = 0;
	virtual void shutdown() = 0;
	virtual void createSurface(void* platformWindowHandle, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<graphicsSurface>& outSurface) = 0;
	virtual void destroySurface(std::shared_ptr<graphicsSurface>& surface) = 0;
	virtual void resizeSurface(graphicsSurface* surface, uint32_t width, uint32_t height) = 0;
	virtual void setSurfaceUseVSync(graphicsSurface* surface, const bool inUseVSync) = 0;
	virtual void beginFrame() = 0;
	virtual void render(const uint32_t numSurfaces, graphicsSurface* const* surfaces, const uint32_t renderDataCount, const sRenderData* const* renderData, const matrix4x4* const viewProjection) = 0;
	virtual void endFrame(const uint32_t numRenderedSurfaces, graphicsSurface* const* renderedSurfaces) = 0;
	//virtual void loadMesh(const size_t vertexCount, const sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, sMeshResources& outMeshResources) = 0;
	virtual void loadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const sVertexPos3Norm3Col4UV2(* const vertices)[], const size_t* const indexCounts, const uint32_t(* const indices)[], sMeshResources** const outMeshResources) = 0;
};
