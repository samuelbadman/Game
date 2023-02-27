#pragma once

#include "platform/graphics/graphicsSurface.h"

class direct3d12Surface : public graphicsSurface
{
public:
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> renderTargetViews;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};
	bool useVSync = false;
};
