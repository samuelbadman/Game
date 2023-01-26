#pragma once

#if defined(PLATFORM_WIN32)

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
	static Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	static Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
	static Microsoft::WRL::ComPtr<ID3D12Device8> device;
	static Microsoft::WRL::ComPtr<ID3D12CommandQueue> graphicsQueue;
	static Microsoft::WRL::ComPtr<ID3D12CommandQueue> computeQueue;
	static Microsoft::WRL::ComPtr<ID3D12CommandQueue> copyQueue;
	static sDescriptorSizes descriptorSizes;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> graphicsCommandAllocators;
	static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> graphicsCommandList;
	static Microsoft::WRL::ComPtr<ID3D12Fence> graphicsFence;
	static uint64_t graphicsFenceValue;
	static std::vector<uint64_t> graphicsFenceValues;
	static HANDLE eventHandle;
	static uint32_t currentBackBufferIndex;

	static Microsoft::WRL::ComPtr<IDxcLibrary> dxcLibrary;
	static Microsoft::WRL::ComPtr<IDxcCompiler> dxcCompiler;

	static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig;
	static Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

public:
	static void init(bool useWarp, uint32_t inBackBufferCount);
	static void shutdown();
	static void createSurface(void* hwnd, uint32_t width, uint32_t height, std::shared_ptr<class graphicsSurface>& outSurface);
	static void destroySurface(std::shared_ptr<class graphicsSurface>& surface);
	static void resizeSurface(class graphicsSurface* surface, uint32_t width, uint32_t height);
	static void render(const uint32_t numSurfaces, class graphicsSurface* const * surfaces, const bool useVSync);

private:
	static void compileShader(LPCWSTR file, LPCWSTR entryPoint, LPCWSTR targetProfile, Microsoft::WRL::ComPtr<IDxcBlob>& outDxcBlob);
	static void waitForGPU();
	static void recordSurface(const class graphicsSurface* surface, ID3D12GraphicsCommandList6* commandList);
	static void presentSurface(const class graphicsSurface* surface, const bool useVSync, const bool tearingSupported);
};

#endif // PLATFORM_WIN32