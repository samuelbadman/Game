#include "pch.h"
#include "vulkanGraphics.h"

void vulkanGraphics::init(bool useWarp, uint32_t inBackBufferCount)
{
}

void vulkanGraphics::shutdown()
{
}

void vulkanGraphics::createSurface(void* hwnd, uint32_t width, uint32_t height, std::shared_ptr<class graphicsSurface>& outSurface)
{
}

void vulkanGraphics::destroySurface(std::shared_ptr<class graphicsSurface>& surface)
{
}

void vulkanGraphics::resizeSurface(graphicsSurface* surface, uint32_t width, uint32_t height)
{
}

void vulkanGraphics::render(const uint32_t numSurfaces, const graphicsSurface* const* surfaces, const bool useVSync, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const class matrix4x4* const viewProjection)
{
}

void vulkanGraphics::loadMesh(const size_t vertexCount, const sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, sMeshResources& outMeshResources)
{
}
