#pragma once

struct sDescriptorSizes
{
	UINT rtvDescriptorSize;
	UINT dsvDescriptorSize;
	UINT cbv_srv_uavDescriptorSize;
	UINT samplerDescriptorSize;
};

// Constant buffer implemented as a ring buffer, storing versions of constant data at 256 byte intervals
class direct3d12ConstantBuffer
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
	UINT64 heapWidth = 0;
	UINT8* bufferStartPointer = nullptr;
	uint32_t bufferPositionIndex = 0;
	uint32_t dataStepRate = 0; // Number of 256 byte steps to move the buffer pointer through the heap to get to the start of the next constant data version

public:
	void init(ID3D12Device8* device, const UINT64 inHeapWidth, const size_t constantDataWidth);
	void shutdown();

	// Overwrites the version of constant data currently pointed to internally
	void update(const void* const src, const size_t size);

	// Returns the gpu virtual address for the version of constant data currently pointed to internally
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress();

	// Moves the internal pointer to the next constant data version
	void increment();

private:
	// Gets the amount the buffer start pointer needs to be offset into the heap to point at the start of the current constant data version
	uint32_t getOffsetToCurrentPosition();
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
	static uint32_t currentFrameIndex;

	static Microsoft::WRL::ComPtr<IDxcLibrary> dxcLibrary;
	static Microsoft::WRL::ComPtr<IDxcCompiler> dxcCompiler;

	static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	static Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

	static std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resourceStore;
	static std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewStore;
	static std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewStore;

	static direct3d12ConstantBuffer objectConstantBuffer;
	static direct3d12ConstantBuffer cameraConstantBuffer;

public:
	static void init(bool useWarp, uint32_t inBackBufferCount);
	static void shutdown();
	static void createSurface(void* hwnd, uint32_t width, uint32_t height, std::shared_ptr<class graphicsSurface>& outSurface);
	static void destroySurface(std::shared_ptr<class graphicsSurface>& surface);
	static void resizeSurface(class graphicsSurface* surface, uint32_t width, uint32_t height);
	static void render(const uint32_t numSurfaces, const class graphicsSurface* const* surfaces, const bool useVSync, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const class matrix4x4* const viewProjection);
	static void loadMesh(const size_t vertexCount, const struct sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, struct sMeshResources& outMeshResources);

private:
	static void loadShader(const std::string& shaderSourceFile, LPCWSTR entryPoint, LPCWSTR targetProfile, std::vector<uint8_t>& outBuffer);
	static void compileShader(LPCWSTR file, LPCWSTR entryPoint, LPCWSTR targetProfile, Microsoft::WRL::ComPtr<IDxcBlob>& outDxcBlob);
	// Waits on the CPU thread for all GPU work to finish
	static void waitForGPU();
	static void recordSurface(const class graphicsSurface* surface, ID3D12GraphicsCommandList6* commandList, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const class matrix4x4* const viewProjection);
	static void presentSurface(const class graphicsSurface* surface, const bool useVSync, const bool tearingSupported);
	static void createDefaultBufferAndRecordCopyCommand(ID3D12GraphicsCommandList6* commandList, ID3D12Resource* copySrcBffer, UINT64 width, size_t& outDefaultBufferResourceHandle);
	static void createVertexBufferView(const size_t vertexBufferResourceHandle, const UINT vertexStride, const UINT bufferWidth, size_t& outVertexBufferViewHandle);
	static void createIndexBufferView(const size_t indexBufferResourceHandle, const DXGI_FORMAT format, const UINT bufferWidth, size_t& outIndexBufferViewHandle);
};

