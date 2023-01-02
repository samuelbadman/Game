#pragma once

#include "renderer.h"

#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d12.h>
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")

class d3d12Renderer : public renderer
{
private:
	static constexpr D3D_FEATURE_LEVEL minSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

private:
	Microsoft::WRL::ComPtr<ID3D12Device9> mainDevice = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> mainAdapter = nullptr;

public:
	// Renderer interface
	virtual rendererPlatform getPlatform() const final { return rendererPlatform::direct3d12; }
	virtual bool init(const rendererInitSettings& settings) final;
	virtual bool shutdown() final;

private:
	bool enableDebugLayer(const bool enableGPUValidation,
		const D3D12_GPU_BASED_VALIDATION_FLAGS gpuBasedValidationFlags,
		const bool enableSynchonizedCommandQueueValidation) const;
	bool reportLiveObjects() const;
	IDXGIAdapter4* enumerateAdapters(IDXGIFactory7* factory) const;

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
