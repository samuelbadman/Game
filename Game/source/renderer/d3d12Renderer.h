#pragma once

#include "renderer.h"

struct descriptorIncrementSizes
{
	uint32_t cbv_srv_uav = 0;
	uint32_t rtv = 0;
	uint32_t dsv = 0;
	uint32_t sampler = 0;
};

class renderContext
{
private:
	renderCommand::commandContext context = renderCommand::commandContext::unknown;
	class d3d12Renderer* owner = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> commandList = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> commandAllocators;
	uint64_t currentFenceValue = 0;
	std::vector<uint64_t> inFlightFenceValues;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent = nullptr;

public:
	bool init(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type, class d3d12Renderer* renderer, uint32_t inFlightFrameCount);
	bool shutdown();
	bool flush();
	bool waitOnCallingCPUThreadForFrame(uint32_t frameIndex);
	void receiveCommand(const renderCommand& command);

	// Command implementation methods
	void command_BeginFrame_Impl(const renderCommand_beginFrame& command);
	void command_EndFrame_Impl(const renderCommand_endFrame& command);
};

class d3d12Renderer : public renderer
{
public:
	static constexpr D3D_FEATURE_LEVEL minimumSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

private:
	Microsoft::WRL::ComPtr<ID3D12Device8> mainDevice = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> mainAdapter = nullptr;

	descriptorIncrementSizes descriptorSizes = {};

	renderContext graphicsContext = {};

public:
	// Renderer interface
	virtual rendererPlatform getPlatform() const final { return rendererPlatform::direct3d12; }
	virtual bool init(const rendererInitSettings& settings) final;
	virtual bool shutdown() final;
	virtual void submitRenderCommand(const renderCommand& command) final;
};
