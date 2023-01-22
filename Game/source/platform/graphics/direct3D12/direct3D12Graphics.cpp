#include "pch.h"

#if defined(PLATFORM_WIN32)

#include "direct3d12Graphics.h"
#include "platform/framework/platformMessageBox.h"

#define fatalIfFailed(x) if(FAILED(x)) platformMessageBoxFatal("hresult failed.");

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
		platformMessageBoxFatal("direct3d12Graphics: did not find any suitable adapter");
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
		platformMessageBoxFatal("direct3d12Graphics::createDevice: failed to cast device to info queue.Could not enable device debug info.");
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
		platformMessageBoxFatal("direct3d12Graphics::createDevice: failed to push queue filter. Could not enable device debug info.");
	}
#endif //_DEBUG
}

static sDescriptorSizes getDescriptorSizes(ID3D12Device8* device)
{
	sDescriptorSizes sizes;
	sizes.rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	sizes.dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	sizes.cbv_srv_uavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	sizes.samplerDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	return sizes;
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
	ComPtr<IDXGISwapChain4>& outSwapChain)
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
	ComPtr<ID3D12DescriptorHeap>& outDescriptorHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = type;
	desc.NumDescriptors = descriptorCount;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	fatalIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&outDescriptorHeap)));
}

static void updateRenderTargetViews(ID3D12Device8* device,
	IDXGISwapChain4* swapChain,
	UINT rtvDescriptorSize,
	std::vector<ComPtr<ID3D12Resource>>& renderTargetViews,
	ID3D12DescriptorHeap* rtvDescriptorHeap,
	UINT backBufferCount)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < backBufferCount; ++i)
	{
		fatalIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargetViews[i])));
		device->CreateRenderTargetView(renderTargetViews[i].Get(), nullptr, rtvHandle);
		renderTargetViews[i]->SetName((std::wstring(L"rtv") + std::to_wstring(i)).c_str());
		rtvHandle.ptr += rtvDescriptorSize;
	}
}

static void createCommandAllocator(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator>& outCommandAllocator)
{
	fatalIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&outCommandAllocator)));
}

static void createCommandList(ID3D12Device8* device,
	ID3D12CommandAllocator* commandAllocator, 
	ID3D12PipelineState* initialPipelineState,
	D3D12_COMMAND_LIST_TYPE type,
	ComPtr<ID3D12GraphicsCommandList6>& outCommandList)
{
	fatalIfFailed(device->CreateCommandList(0, type, commandAllocator, initialPipelineState, IID_PPV_ARGS(&outCommandList)));
	fatalIfFailed(outCommandList->Close());
}

static void createFence(ID3D12Device8* device, ComPtr<ID3D12Fence>& outFence)
{
	fatalIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&outFence)));
}

static void createEventHandle(HANDLE& outEventHandle)
{
	outEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (outEventHandle == nullptr)
	{
		platformMessageBoxFatal("direct3d12Graphics::CreateEvent: failed to create event handle.");
	}
}

ComPtr<IDXGIFactory7> direct3d12Graphics::dxgiFactory;
ComPtr<IDXGIAdapter4> direct3d12Graphics::adapter;
ComPtr<ID3D12Device8> direct3d12Graphics::device;
ComPtr<ID3D12CommandQueue> direct3d12Graphics::graphicsQueue;
ComPtr<ID3D12CommandQueue> direct3d12Graphics::computeQueue;
ComPtr<ID3D12CommandQueue> direct3d12Graphics::copyQueue;
ComPtr<IDXGISwapChain4> direct3d12Graphics::swapChain;
sDescriptorSizes direct3d12Graphics::descriptorSizes = {};
std::vector<ComPtr<ID3D12Resource>> direct3d12Graphics::renderTargetViews;
ComPtr<ID3D12DescriptorHeap> direct3d12Graphics::rtvDescriptorHeap;
std::vector<ComPtr<ID3D12CommandAllocator>> direct3d12Graphics::graphicsCommandAllocators;
ComPtr<ID3D12GraphicsCommandList6> direct3d12Graphics::graphicsCommandList;
ComPtr<ID3D12Fence> direct3d12Graphics::graphicsFence;
uint64_t direct3d12Graphics::graphicsFenceValue = 0;
std::vector<uint64_t> direct3d12Graphics::graphicsFenceValues;
HANDLE direct3d12Graphics::eventHandle = nullptr;
uint32_t direct3d12Graphics::currentBackBufferIndex = 0;

void direct3d12Graphics::init(bool useWarp, void* hwnd, uint32_t width, uint32_t height, uint32_t backBufferCount)
{
	enableDebugLayer();
	createDxgiFactory(dxgiFactory);
	getAdapter(dxgiFactory.Get(), false, adapter);
	createDevice(adapter.Get(), device);
	descriptorSizes = getDescriptorSizes(device.Get());
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, graphicsQueue);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE, computeQueue);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_COPY, copyQueue);

	createSwapChain(dxgiFactory.Get(), graphicsQueue.Get(), static_cast<HWND>(hwnd), width, height, backBufferCount, swapChain);
	createDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, backBufferCount, false, rtvDescriptorHeap);
	renderTargetViews.resize(static_cast<size_t>(backBufferCount));
	updateRenderTargetViews(device.Get(), swapChain.Get(), descriptorSizes.rtvDescriptorSize, renderTargetViews, rtvDescriptorHeap.Get(), backBufferCount);

	graphicsCommandAllocators.resize(static_cast<size_t>(backBufferCount));
	for (ComPtr<ID3D12CommandAllocator>& commandAllocator : graphicsCommandAllocators)
	{
		createCommandAllocator(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator);
	}
	createCommandList(device.Get(), graphicsCommandAllocators[0].Get(), nullptr, D3D12_COMMAND_LIST_TYPE_DIRECT, graphicsCommandList);

	createFence(device.Get(), graphicsFence);
	graphicsFenceValue = 0;
	graphicsFenceValues.resize(static_cast<size_t>(backBufferCount), graphicsFenceValue);
	createEventHandle(eventHandle);
}

void direct3d12Graphics::shutdown()
{
	waitForGPU();
}

void direct3d12Graphics::resize(uint32_t width, uint32_t height)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	fatalIfFailed(swapChain->GetDesc(&swapChainDesc));

	// Don't resize if the width and height have not changed
	if (swapChainDesc.BufferDesc.Width != width || swapChainDesc.BufferDesc.Height != height)
	{
		// Don't allow 0 size swap chain back buffers
		width = std::max(1u, width);
		height = std::max(1u, height);

		// Wait for all frames to finish executing on the GPU
		waitForGPU();

		// Release render target view resources
		const size_t backBufferCount = renderTargetViews.size();
		for (size_t i = 0; i < backBufferCount; ++i)
		{
			renderTargetViews[i].Reset();
			graphicsFenceValues[i] = graphicsFenceValues[currentBackBufferIndex];
		}

		// Resize the swap chain back buffers
		fatalIfFailed(swapChain->ResizeBuffers(static_cast<UINT>(backBufferCount), width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		// Update current back buffer index
		currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

		// Update render target view resources with new back buffers
		updateRenderTargetViews(device.Get(), swapChain.Get(), descriptorSizes.rtvDescriptorSize, renderTargetViews, rtvDescriptorHeap.Get(), static_cast<UINT>(backBufferCount));
	}
}

void direct3d12Graphics::render(const bool useVSync)
{
	// Wait for the previous frame to finish on the GPU
	waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[currentBackBufferIndex]);

	// Get frame resources
	ID3D12CommandAllocator* const graphicsCommandAllocator = graphicsCommandAllocators[currentBackBufferIndex].Get();
	ID3D12Resource* const backBuffer = renderTargetViews[currentBackBufferIndex].Get();

	// Start recording command list
	fatalIfFailed(graphicsCommandAllocator->Reset());
	fatalIfFailed(graphicsCommandList->Reset(graphicsCommandAllocator, nullptr));

	D3D12_RESOURCE_BARRIER backBufferResourceStartTransitionBarrier = {};
	backBufferResourceStartTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBufferResourceStartTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBufferResourceStartTransitionBarrier.Transition.pResource = backBuffer;
	backBufferResourceStartTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBufferResourceStartTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	backBufferResourceStartTransitionBarrier.Transition.Subresource = 0;

	graphicsCommandList->ResourceBarrier(1, &backBufferResourceStartTransitionBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_rtvDescriptorHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cpu_rtvDescriptorHandle.ptr += (descriptorSizes.rtvDescriptorSize * currentBackBufferIndex);
	const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.4f, 1.0f };
	graphicsCommandList->ClearRenderTargetView(cpu_rtvDescriptorHandle, clearColor, 0, nullptr);

	D3D12_RESOURCE_BARRIER backBufferResourceEndTransitionBarrier = {};
	backBufferResourceEndTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBufferResourceEndTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBufferResourceEndTransitionBarrier.Transition.pResource = backBuffer;
	backBufferResourceEndTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	backBufferResourceEndTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBufferResourceEndTransitionBarrier.Transition.Subresource = 0;

	graphicsCommandList->ResourceBarrier(1, &backBufferResourceEndTransitionBarrier);

	// Stop recording command list
	fatalIfFailed(graphicsCommandList->Close());

	// Execute command lists
	ID3D12CommandList* graphicsExecuteLists[1] = { graphicsCommandList.Get() };
	graphicsQueue->ExecuteCommandLists(_countof(graphicsExecuteLists), graphicsExecuteLists);

	// Present
	static bool tearingSupported = checkTearingSupport(dxgiFactory.Get());
	fatalIfFailed(swapChain->Present(useVSync ? 1 : 0, ((tearingSupported) && (!useVSync)) ? DXGI_PRESENT_ALLOW_TEARING : 0));
	currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

	// Signal end frame. Must be done after present as flip discard swap effect is being used and this presents without blocking CPU thread
	++graphicsFenceValue;
	graphicsFenceValues[currentBackBufferIndex] = graphicsFenceValue;
	fatalIfFailed(graphicsQueue->Signal(graphicsFence.Get(), graphicsFenceValues[currentBackBufferIndex]));
}

void direct3d12Graphics::waitForGPU()
{
	// Wait for all frames to finish executing on the GPU
	const size_t backBufferCount = renderTargetViews.size();
	for (size_t i = 0; i < backBufferCount; ++i)
	{
		waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[i]);
	}
}

void direct3d12Graphics::waitForFence(ID3D12Fence* fence, HANDLE inEventHandle, uint64_t value)
{
	if (fence->GetCompletedValue() < value)
	{
		fatalIfFailed(fence->SetEventOnCompletion(value, inEventHandle));
		if (WaitForSingleObject(inEventHandle, maxFenceWaitDurationMs) == WAIT_FAILED)
		{
			platformMessageBoxFatal("direct3d12Graphics::waitForFence: failed to wait for single object.");
		}
	}
}

#endif // PLATFORM_WIN32
