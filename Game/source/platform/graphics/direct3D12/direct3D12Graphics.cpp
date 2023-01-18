#include "pch.h"
#include "direct3d12Graphics.h"
#include "platform/framework/win32/win32MessageBox.h"

using namespace Microsoft::WRL;

#define fatalIfFailed(x) if(FAILED(x)) win32MessageBox::messageBoxFatal("hresult failed.");

static void enableDebugLayer()
{
#if defined(_DEBUG)
	// Enable the d3d12 debug layer
	ComPtr<ID3D12Debug> debugInterface;
	fatalIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif // _DEBUG
}

static void createDxgiFactory(ComPtr<IDXGIFactory7>& outFactory)
{
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // _DEBUG

	fatalIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&outFactory)));
}

static void getAdapter(IDXGIFactory7* dxgiFactory, bool useWarp, ComPtr<IDXGIAdapter4>& outAdapter)
{
	if (useWarp)
	{
		// Get the software rasterizer adapter
		ComPtr<IDXGIAdapter1> adapter1;
		fatalIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter1)));
		fatalIfFailed(adapter1.As(&outAdapter));
	}
	else
	{
		// Get hardware adapters in descending order of performance. Highest performance adapter is the first loop iteration.
		for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&outAdapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			// Select the first adapter that supports the minimum feature level.
			if (SUCCEEDED(D3D12CreateDevice(outAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
			{
				return;
			}
		}

		// No adapters were found that meet minimum requirements
		win32MessageBox::messageBoxFatal("direct3d12Graphics: did not find any suitable adapter");
	}
}

static void createDevice(IDXGIAdapter4* adapter, ComPtr<ID3D12Device8>& outDevice)
{
	fatalIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&outDevice)));

#if defined(_DEBUG)
	// Enable device debug info
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> InfoQueue;
	if (FAILED(outDevice.As(&InfoQueue)))
	{
		win32MessageBox::messageBoxFatal("direct3d12Graphics::createDevice: failed to cast device to info queue.Could not enable device debug info.");
	}

	InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

	D3D12_MESSAGE_SEVERITY Severities[] =
	{
		D3D12_MESSAGE_SEVERITY_INFO
	};

	D3D12_MESSAGE_ID DenyIDs[] =
	{
		D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
		D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
		D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		// Workarounds for debug layer errors on hybrid-graphics systems
		D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
		D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
	};

	D3D12_INFO_QUEUE_FILTER QueueFilter = {};
	QueueFilter.DenyList.NumSeverities = _countof(Severities);
	QueueFilter.DenyList.pSeverityList = Severities;
	QueueFilter.DenyList.NumIDs = _countof(DenyIDs);
	QueueFilter.DenyList.pIDList = DenyIDs;

	if (FAILED(InfoQueue->PushStorageFilter(&QueueFilter)))
	{
		win32MessageBox::messageBoxFatal("direct3d12Graphics::createDevice: failed to push queue filter. Could not enable device debug info.");
	}
#endif //_DEBUG
}

static void createCommandQueue(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandQueue>& outCommandQueue)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	fatalIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&outCommandQueue)));
}

static bool checkTearingSupport(IDXGIFactory7* dxgiFactory)
{
	BOOL allowTearing = FALSE;

	if (FAILED(dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
	{
		allowTearing = FALSE;
	}

	return (allowTearing == TRUE);
}

static void createSwapChain(IDXGIFactory7* dxgiFactory, 
	ID3D12CommandQueue* directCommandQueue, 
	HWND hwnd, 
	UINT width, 
	UINT height, 
	UINT bufferCount,
	ComPtr<IDXGISwapChain4> outSwapChain)
{
	// Create swap chain
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = false;
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = bufferCount;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = checkTearingSupport(dxgiFactory) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	fatalIfFailed(dxgiFactory->CreateSwapChainForHwnd(directCommandQueue, hwnd, &desc, nullptr, nullptr, &swapChain1));

	// Disable alt + enter fullscreen shortcut
	fatalIfFailed(dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

	// Convert swap chain 1 interface to swap chain 4 interface
	fatalIfFailed(swapChain1.As(&outSwapChain));
}

static void createDescriptorHeap(ID3D12Device8* device, 
	D3D12_DESCRIPTOR_HEAP_TYPE type, 
	UINT descriptorCount, 
	bool shaderVisible, 
	ComPtr<ID3D12DescriptorHeap> outDescriptorHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = type;
	desc.NumDescriptors = descriptorCount;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	fatalIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&outDescriptorHeap)));
}

static ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
static ComPtr<IDXGIAdapter4> adapter = nullptr;
static ComPtr<ID3D12Device8> device = nullptr;
static ComPtr<ID3D12CommandQueue> graphicsQueue = nullptr;
static ComPtr<ID3D12CommandQueue> computeQueue = nullptr;
static ComPtr<ID3D12CommandQueue> copyQueue = nullptr;
static ComPtr<IDXGISwapChain4> swapChain = nullptr;
static ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;

void direct3d12Graphics::init(bool useWarp, HWND hwnd, uint32_t width, uint32_t height, uint32_t backBufferCount)
{
	enableDebugLayer();
	createDxgiFactory(dxgiFactory);
	getAdapter(dxgiFactory.Get(), false, adapter);
	createDevice(adapter.Get(), device);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, graphicsQueue);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE, computeQueue);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_COPY, copyQueue);
	createSwapChain(dxgiFactory.Get(), graphicsQueue.Get(), hwnd, width, height, backBufferCount, swapChain);
	createDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, backBufferCount, false, rtvDescriptorHeap);
}
