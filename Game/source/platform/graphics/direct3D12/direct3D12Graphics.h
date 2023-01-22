#pragma once

#if defined(PLATFORM_WIN32)

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
	static ComPtr<IDXGIFactory7> dxgiFactory;
	static ComPtr<IDXGIAdapter4> adapter;
	static ComPtr<ID3D12Device8> device;
	static ComPtr<ID3D12CommandQueue> graphicsQueue;
	static ComPtr<ID3D12CommandQueue> computeQueue;
	static ComPtr<ID3D12CommandQueue> copyQueue;
	static ComPtr<IDXGISwapChain4> swapChain;
	static sDescriptorSizes descriptorSizes;
	static std::vector<ComPtr<ID3D12Resource>> renderTargetViews;
	static ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	static std::vector<ComPtr<ID3D12CommandAllocator>> graphicsCommandAllocators;
	static ComPtr<ID3D12GraphicsCommandList6> graphicsCommandList;
	static ComPtr<ID3D12Fence> graphicsFence;
	static uint64_t graphicsFenceValue;
	static std::vector<uint64_t> graphicsFenceValues;
	static HANDLE eventHandle;
	static uint32_t currentBackBufferIndex;

public:
	static void init(bool useWarp, void* hwnd, uint32_t width, uint32_t height, uint32_t backBufferCount);
	static void shutdown();
	static void resize(uint32_t width, uint32_t height);
	static void render(const bool useVSync);

private:
	static void waitForGPU();
};

#endif // PLATFORM_WIN32