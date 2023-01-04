#include "pch.h"
#include "renderer.h"
#include "d3d12Renderer.h"

std::unique_ptr<renderDevice> renderDevice::create(const rendererPlatform platform)
{
    switch (platform)
    {
    case rendererPlatform::direct3d12: return std::make_unique<d3d12RenderDevice>();
    }
    return nullptr;
}
