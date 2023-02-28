#pragma once

#include "platform/graphics/graphics.h"

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

class direct3d12Graphics : public graphics
{
private:
	uint32_t backBufferCount = 0;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
	Microsoft::WRL::ComPtr<ID3D12Device8> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> graphicsQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> computeQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> copyQueue;
	sDescriptorSizes descriptorSizes = {};
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> graphicsCommandAllocators;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> graphicsCommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence> graphicsFence;
	uint64_t graphicsFenceValue = 0;
	std::vector<uint64_t> graphicsFenceValues;
	HANDLE eventHandle = {};
	uint32_t currentFrameIndex = 0;

	Microsoft::WRL::ComPtr<IDxcLibrary> dxcLibrary;
	Microsoft::WRL::ComPtr<IDxcCompiler> dxcCompiler;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resourceStore;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewStore;
	std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewStore;

	direct3d12ConstantBuffer objectConstantBuffer = {};
	direct3d12ConstantBuffer cameraConstantBuffer = {};

public:
	~direct3d12Graphics() final = default;

public:
	// Graphics interface
	void init(const bool useWarp, uint32_t inBackBufferCount) final;
	void shutdown() final;
	void createSurface(void* hwnd, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<class graphicsSurface>& outSurface) final;
	void destroySurface(std::shared_ptr<class graphicsSurface>& surface) final;
	void resizeSurface(class graphicsSurface* surface, uint32_t width, uint32_t height) final;
	void setSurfaceUseVSync(class graphicsSurface* surface, const bool inUseVSync) final;
	void beginFrame() final;
	void render(const uint32_t numSurfaces, const class graphicsSurface* const* surfaces, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const class matrix4x4* const viewProjection) final;
	void endFrame(const uint32_t numRenderedSurfaces, const class graphicsSurface* const* renderedSurfaces) final;
	//void loadMesh(const size_t vertexCount, const struct sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, struct sMeshResources& outMeshResource) final;
	void loadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const struct sVertexPos3Norm3Col4UV2(*vertices)[], const size_t* const indexCounts, const uint32_t(*indices)[], struct sMeshResources** const outMeshResources) final;

private:
	void loadShader(const std::string& shaderSourceFile, LPCWSTR entryPoint, LPCWSTR targetProfile, std::vector<uint8_t>& outBuffer);
	void compileShader(LPCWSTR file, LPCWSTR entryPoint, LPCWSTR targetProfile, Microsoft::WRL::ComPtr<IDxcBlob>& outDxcBlob);
	// Waits on the CPU thread for all GPU work to finish
	void waitForGPU();
	void recordSurface(const class direct3d12Surface* surface, ID3D12GraphicsCommandList6* commandList, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const class matrix4x4* const viewProjection);
	void presentSurface(const class direct3d12Surface* surface, const bool useVSync, const bool tearingSupported);
	void createDefaultBufferAndRecordCopyCommand(ID3D12GraphicsCommandList6* commandList, ID3D12Resource* copySrcBffer, UINT64 width, size_t& outDefaultBufferResourceHandle);
	void createVertexBufferView(const size_t vertexBufferResourceHandle, const UINT vertexStride, const UINT bufferWidth, size_t& outVertexBufferViewHandle);
	void createIndexBufferView(const size_t indexBufferResourceHandle, const DXGI_FORMAT format, const UINT bufferWidth, size_t& outIndexBufferViewHandle);
};

