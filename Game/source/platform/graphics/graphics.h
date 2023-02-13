#pragma once

#include "graphicsApi.h"

extern void graphicsInit(const eGraphicsApi graphicsApi, const bool softwareRenderer, const uint32_t backBufferCount);
extern void graphicsShutdown();
extern void graphicsCreateSurface(void* platformWindowHandle, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<class graphicsSurface>& outSurface);
extern void graphicsDestroySurface(std::shared_ptr<class graphicsSurface>& surface);
extern void graphicsResizeSurface(class graphicsSurface* surface, uint32_t width, uint32_t height);
extern void graphicsSetSurfaceUseVSync(class graphicsSurface* surface, const bool inUseVSync);
extern void graphicsBeginFrame();
extern void graphicsRender(const uint32_t numSurfaces, const class graphicsSurface* const* surfaces, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const class matrix4x4* const viewProjection);
extern void graphicsEndFrame(const uint32_t numRenderedSurfaces, const class graphicsSurface* const* renderedSurfaces);
//extern void graphicsLoadMesh(const size_t vertexCount, const struct sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, struct sMeshResources& outMeshResources);
extern void graphicsLoadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const struct sVertexPos3Norm3Col4UV2(* const vertices)[], const size_t* const indexCounts, const uint32_t(* const indices)[], struct sMeshResources** const outMeshResources);
