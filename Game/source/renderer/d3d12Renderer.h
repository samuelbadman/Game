#pragma once

#include "renderer.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

class d3d12Renderer : public renderer
{
private:
	Microsoft::WRL::ComPtr<ID3D12Device9> mainDevice;

public:
	virtual rendererPlatform getPlatform() const final { return rendererPlatform::direct3d12; }
	virtual bool init() final;
	virtual bool shutdown() final;
};