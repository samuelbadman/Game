#pragma once

#include "renderer.h"

struct descriptorIncrementSizes
{
	uint32_t cbv_srv_uav = 0;
	uint32_t rtv = 0;
	uint32_t dsv = 0;
	uint32_t sampler = 0;
};

class renderEngine
{
private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> commandList = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> commandAllocators;
	uint64_t currentFenceValue = 0;
	std::vector<uint64_t> inFlightFenceValues;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent = nullptr;

public:
	bool init(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type, uint32_t inFlightFrameCount);
	bool shutdown();
	bool flush();
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

	renderEngine graphicsEngine = {};

public:
	// Renderer interface
	virtual rendererPlatform getPlatform() const final { return rendererPlatform::direct3d12; }
	virtual bool init(const rendererInitSettings& settings) final;
	virtual bool shutdown() final;
};
