#pragma once

#include "renderer.h"

struct descriptorIncrementSizes
{
	uint32_t cbv_srv_uav = 0;
	uint32_t rtv = 0;
	uint32_t dsv = 0;
	uint32_t sampler = 0;
};

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
	void submitRenderContexts(renderContext*const* contexts);
};

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
};

class d3d12RenderDevice : public renderDevice
{
public:
	static constexpr D3D_FEATURE_LEVEL minimumSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

private:
	Microsoft::WRL::ComPtr<ID3D12Device8> mainDevice = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> mainAdapter = nullptr;

	descriptorIncrementSizes descriptorSizes = {};

	d3d12HardwareQueue graphicsQueue = {};

	uint64_t currentFenceValue = 0;
	std::vector<uint64_t> inFlightFenceValues;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent = nullptr;

public:
	// Render device interface
	virtual rendererPlatform getPlatform() const final { return rendererPlatform::direct3d12; }
	virtual bool init(const renderDeviceInitSettings& settings) final;
	virtual bool shutdown() final;
	virtual bool flush() final;
	virtual void submitRenderContexts(const renderCommand::commandContext commandContext, renderContext*const* contexts) final;
};
