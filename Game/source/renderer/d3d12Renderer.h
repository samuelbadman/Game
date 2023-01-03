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
};

class d3d12Renderer : public renderer
{
private:
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

private:
	// TODO Make these free functions inside d3d12Renderer.cpp including release
	bool enableDebugLayer(const bool enableGPUValidation,
		const D3D12_GPU_BASED_VALIDATION_FLAGS gpuBasedValidationFlags,
		const bool enableSynchonizedCommandQueueValidation) const;
	bool reportLiveObjects() const;
	IDXGIAdapter4* enumerateAdapters(IDXGIFactory7* factory) const;
	D3D_FEATURE_LEVEL getAdapterMaximumFeatureLevel(IDXGIAdapter4* adapter) const;
	bool enableDeviceDebugInfo(const Microsoft::WRL::ComPtr<ID3D12Device8>& device) const;
	bool getTearingSupport(IDXGIFactory7* factory) const;

	template<typename T>
	void release(T*& resource) const;
};

template<typename T>
inline void d3d12Renderer::release(T*& resource) const
{
	if (resource != nullptr)
	{
		resource->Release();
		resource = nullptr;
	}
}
