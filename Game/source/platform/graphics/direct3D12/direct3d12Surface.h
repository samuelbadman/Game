#pragma once

#if defined(PLATFORM_WIN32)

using namespace Microsoft::WRL;

struct sDirect3d12Surface
{
	ComPtr<IDXGISwapChain4> swapChain;
	std::vector<ComPtr<ID3D12Resource>> renderTargetViews;
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	ComPtr<ID3D12Resource> depthStencilView;
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
};

#endif // PLATFORM_WIN32