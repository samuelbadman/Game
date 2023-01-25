#pragma once

#if defined(PLATFORM_WIN32)

using namespace Microsoft::WRL;

class graphicsSurface
{
public:
	ComPtr<IDXGISwapChain4> swapChain;
	std::vector<ComPtr<ID3D12Resource>> renderTargetViews;
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	ComPtr<ID3D12Resource> depthStencilView;
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};
};

#endif // PLATFORM_WIN32