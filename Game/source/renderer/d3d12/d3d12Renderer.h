#pragma once

#include "renderer/renderer.h"

// ---------------------------------------------
// Structs
// ---------------------------------------------
struct d3d12DescriptorIncrementSizes
{
	UINT cbv_srv_uav = 0;
	UINT rtv = 0;
	UINT dsv = 0;
	UINT sampler = 0;
};

// ---------------------------------------------
// Descriptor heap
// ---------------------------------------------
class d3d12DescriptorHeap
{
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap = nullptr;

public:
	bool init(ID3D12Device8* const device, const UINT descriptorCount,
		const D3D12_DESCRIPTOR_HEAP_TYPE type, const bool shaderVisible);
	void shutdown();
	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandleForDescriptorAtHeapStart() const;
};

// ---------------------------------------------
// Hardware queue
// ---------------------------------------------
class d3d12HardwareQueue
{
private:
	renderCommand::commandContext queueContext = renderCommand::commandContext::unknown;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue = nullptr;
	std::vector<ID3D12CommandList*> commandListSubmissions;

public:
	bool init(ID3D12Device8* const device, const D3D12_COMMAND_LIST_TYPE type, 
		const size_t graphicsContextSubmissionsPerFrameCount);
	void shutdown();
	void submitRenderContexts(const uint32_t numContexts, renderContext*const* contexts);
	ID3D12CommandQueue* GetCommandQueue() const { return queue.Get(); }
};

// ---------------------------------------------
// Swap chain
// ---------------------------------------------
class d3d12SwapChain : public swapChain
{
private:
	// Swap chain
	Microsoft::WRL::ComPtr<IDXGISwapChain4> dxgiSwapChain = nullptr;

	// Resources
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> rtvs;
	Microsoft::WRL::ComPtr<ID3D12Resource> dsv = nullptr;

	// Resource descriptor heaps
	d3d12DescriptorHeap rtDescriptorHeap = {};
	d3d12DescriptorHeap dsDescriptorHeap = {};

public:
	// Swap chain interface
	virtual rendererPlatform getPlatform() const final { return rendererPlatform::direct3d12; }

public:
	bool init(IDXGIFactory7* const factory, ID3D12CommandQueue* const directCommandQueue,
		ID3D12Device8* const device,
		const uint32_t width, const uint32_t height,
		const uint32_t backBufferCount, HWND hwnd);
	bool shutdown();
	bool updateBackBufferRTVs(ID3D12Device8* const device, const UINT rtvDescriptorSize);
	bool updateDSV(ID3D12Device8* const device, const UINT64 width, const UINT height);
	bool getSwapChainDesc(DXGI_SWAP_CHAIN_DESC& outSwapChainDesc) const;
	bool resize(ID3D12Device8* const device, const UINT rtDescriptorSize, const UINT64 width, const UINT height,
		const DXGI_SWAP_CHAIN_DESC& swapChainDesc);
};

// ---------------------------------------------
// Render context
// ---------------------------------------------
class d3d12RenderContext : public renderContext
{
private:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> commandList = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> commandAllocators;

public:
	// Render context interface
	virtual rendererPlatform getPlatform() const { return rendererPlatform::direct3d12; }
	virtual void submitRenderCommand(const renderCommand& command) final;

public:
	bool init(ID3D12Device8* const device, const uint8_t inFlightFrameCount, 
		const renderCommand::commandContext commandContext);
	bool shutdown();
	ID3D12GraphicsCommandList6* getCommandList() { return commandList.Get(); }

private:
	// Command implementations
	void renderCommand_beginContext_implementation(const renderCommand_beginContext& command);
	void renderCommand_endContext_implementation(const renderCommand_endContext& command);
};

// ---------------------------------------------
// Render device
// ---------------------------------------------
class d3d12RenderDevice : public renderDevice
{
public:
	static constexpr D3D_FEATURE_LEVEL minimumSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

private:
	Microsoft::WRL::ComPtr<ID3D12Device8> mainDevice = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> mainAdapter = nullptr;

	d3d12DescriptorIncrementSizes descriptorSizes = {};

	d3d12HardwareQueue graphicsQueue = {};

	// Frame synchronization
	uint64_t currentFenceValue = 0;
	std::vector<uint64_t> inFlightFenceValues;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent = nullptr;
	uint32_t currentFrameIndex = 0;

public:
	// Render device interface
	virtual rendererPlatform getPlatform() const final { return rendererPlatform::direct3d12; }
	virtual bool init(const renderDeviceInitSettings& settings) final;
	virtual void shutdown() final;
	virtual bool flush() final;
	virtual void submitRenderContexts(const renderCommand::commandContext commandContext, 
		const uint32_t numContexts, renderContext*const* contexts) final;
	virtual bool createRenderContext(const renderCommand::commandContext commandContext,
		std::unique_ptr<renderContext>& outRenderContext) const final;
	virtual bool destroyRenderContext(std::unique_ptr<renderContext>& outRenderContext) final;
	virtual bool createSwapChain(const swapChainInitSettings& settings, 
		std::unique_ptr<swapChain>& outSwapChain) final;
	virtual bool destroySwapChain(std::unique_ptr<swapChain>& outSwapChain) final;
	virtual bool resizeSwapChain(swapChain* inSwapChain, const uint32_t newWidth, const uint32_t newHeight) final;
};
