#pragma once

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
	static Microsoft::WRL::ComPtr<ID3D12CommandAllocator> copyCommandAllocator;
	static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> copyCommandList;
	static Microsoft::WRL::ComPtr<ID3D12Fence> copyFence;
	static uint64_t copyFenceValue;
	static HANDLE eventHandle;
	static uint32_t currentFrameIndex;

	static Microsoft::WRL::ComPtr<IDxcLibrary> dxcLibrary;
	static Microsoft::WRL::ComPtr<IDxcCompiler> dxcCompiler;

	static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig;
	static Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

	static std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resourceStore;

public:
	static void init(bool useWarp, uint32_t inBackBufferCount);
	static void shutdown();
	static void createSurface(void* hwnd, uint32_t width, uint32_t height, std::shared_ptr<class graphicsSurface>& outSurface);
	static void destroySurface(std::shared_ptr<class graphicsSurface>& surface);
	static void resizeSurface(class graphicsSurface* surface, uint32_t width, uint32_t height);
	static void render(const uint32_t numSurfaces, class graphicsSurface* const * surfaces, const bool useVSync);

	static void loadMesh(const size_t vertexCount,
		const struct sVertexPos3Norm3Col4UV2* const* vertices, 
		const size_t indexCount, 
		const uint32_t* const* indices,
		struct sMeshResources& outMeshResources);

private:
	static void loadShader(const std::string& shaderSourceFile, LPCWSTR entryPoint, LPCWSTR targetProfile, std::vector<uint8_t>& outBuffer);
	static void compileShader(LPCWSTR file, LPCWSTR entryPoint, LPCWSTR targetProfile, Microsoft::WRL::ComPtr<IDxcBlob>& outDxcBlob);
	// Waits on the CPU thread for all GPU work to finish
	static void waitForGPU();
	static void recordSurface(const class graphicsSurface* surface, ID3D12GraphicsCommandList6* commandList);
	static void presentSurface(const class graphicsSurface* surface, const bool useVSync, const bool tearingSupported);
};

