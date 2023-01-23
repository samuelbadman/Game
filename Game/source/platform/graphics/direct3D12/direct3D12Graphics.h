#pragma once

#if defined(PLATFORM_WIN32)

#include "direct3d12Surface.h"

using namespace Microsoft::WRL;

struct sDescriptorSizes
{
	UINT rtvDescriptorSize;
	UINT dsvDescriptorSize;
	UINT cbv_srv_uavDescriptorSize;
	UINT samplerDescriptorSize;
};

class direct3d12Graphics
{
private:
	static uint32_t backBufferCount;
	static ComPtr<IDXGIFactory7> dxgiFactory;
	static ComPtr<IDXGIAdapter4> adapter;
	static ComPtr<ID3D12Device8> device;
	static ComPtr<ID3D12CommandQueue> graphicsQueue;
	static ComPtr<ID3D12CommandQueue> computeQueue;
	static ComPtr<ID3D12CommandQueue> copyQueue;
	static sDescriptorSizes descriptorSizes;
	static std::vector<ComPtr<ID3D12CommandAllocator>> graphicsCommandAllocators;
	static ComPtr<ID3D12GraphicsCommandList6> graphicsCommandList;
	static ComPtr<ID3D12Fence> graphicsFence;
	static uint64_t graphicsFenceValue;
	static std::vector<uint64_t> graphicsFenceValues;
	static HANDLE eventHandle;
	static uint32_t currentBackBufferIndex;

public:
	static void init(bool useWarp, uint32_t inBackBufferCount);
	static void shutdown();
	static sDirect3d12Surface createSurface(void* hwnd, uint32_t width, uint32_t height);
	static void destroySurface(sDirect3d12Surface& surface);
	static void resizeSurface(sDirect3d12Surface& surface, uint32_t width, uint32_t height);
	static void render(const uint32_t numSurfaces, sDirect3d12Surface* const * surfaces, const bool useVSync);

private:
	static void waitForGPU();
	static void recordSurface(const sDirect3d12Surface& surface, ID3D12GraphicsCommandList6* commandList);
};

#endif // PLATFORM_WIN32