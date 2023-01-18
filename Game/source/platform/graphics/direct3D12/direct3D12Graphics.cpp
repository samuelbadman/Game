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

static ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
static ComPtr<IDXGIAdapter4> adapter = nullptr;
static ComPtr<ID3D12Device8> device = nullptr;
static ComPtr<ID3D12CommandQueue> graphicsQueue = nullptr;
static ComPtr<ID3D12CommandQueue> computeQueue = nullptr;
static ComPtr<ID3D12CommandQueue> copyQueue = nullptr;

void direct3d12Graphics::init(bool useWarp)
{
	enableDebugLayer();
	createDxgiFactory(dxgiFactory);
	getAdapter(dxgiFactory.Get(), false, adapter);
	createDevice(adapter.Get(), device);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, graphicsQueue);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE, computeQueue);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_COPY, copyQueue);

}
