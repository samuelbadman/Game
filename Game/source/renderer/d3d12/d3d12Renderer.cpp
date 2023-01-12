#include "pch.h"
#include "d3d12Renderer.h"
#include "log.h"
#include "stringHelper.h"

// ---------------------------------------------
// Free functions
// ---------------------------------------------
static std::string getD3dDescriptorHeapTypeAsString(const D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	switch (type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return "D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV";
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return "D3D12_DESCRIPTOR_HEAP_TYPE_RTV";
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return "D3D12_DESCRIPTOR_HEAP_TYPE_DSV";
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: return "D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER";
	}
	return "INVALID_DESCRIPTOR_HEAP_TYPE";
}

static std::string getD3dFeatureLevelAsString(const D3D_FEATURE_LEVEL featureLevel)
{
	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_9_1:	 return "D3D_FEATURE_LEVEL_9_1";
	case D3D_FEATURE_LEVEL_9_2:	 return "D3D_FEATURE_LEVEL_9_2";
	case D3D_FEATURE_LEVEL_9_3:	 return "D3D_FEATURE_LEVEL_9_3";
	case D3D_FEATURE_LEVEL_10_0: return "D3D_FEATURE_LEVEL_10_0";
	case D3D_FEATURE_LEVEL_10_1: return "D3D_FEATURE_LEVEL_10_1";
	case D3D_FEATURE_LEVEL_11_0: return "D3D_FEATURE_LEVEL_11_0";
	case D3D_FEATURE_LEVEL_11_1: return "D3D_FEATURE_LEVEL_11_1";
	case D3D_FEATURE_LEVEL_12_0: return "D3D_FEATURE_LEVEL_12_0";
	case D3D_FEATURE_LEVEL_12_1: return "D3D_FEATURE_LEVEL_12_1";
	case D3D_FEATURE_LEVEL_12_2: return "D3D_FEATURE_LEVEL_12_2";
	}
	return "INVALID_FEATURE_LEVEL";
}

static std::string getCommandListTypeAsString(const D3D12_COMMAND_LIST_TYPE type)
{
	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT: return "D3D12_COMMAND_LIST_TYPE_DIRECT";
	case D3D12_COMMAND_LIST_TYPE_COMPUTE: return "D3D12_COMMAND_LIST_TYPE_COMPUTE";
	case D3D12_COMMAND_LIST_TYPE_COPY: return "D3D12_COMMAND_LIST_TYPE_COPY";
	case D3D12_COMMAND_LIST_TYPE_BUNDLE: return "D3D12_COMMAND_LIST_TYPE_BUNDLE";
	case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE: return "D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE";
	case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE: return "D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE";
	case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS: return "D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS";
	}
	return "INVALID_COMMAND_LIST_TYPE";
}

static D3D12_COMMAND_LIST_TYPE commandContextToD3d12CommandListType(const renderCommand::commandContext commandContext)
{
	switch (commandContext)
	{
		case renderCommand::commandContext::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}
	return D3D12_COMMAND_LIST_TYPE();
}

static renderCommand::commandContext d3d12CommandListTypeToCommandContext(const D3D12_COMMAND_LIST_TYPE type)
{
	switch (type)
	{
		case D3D12_COMMAND_LIST_TYPE_DIRECT: return renderCommand::commandContext::graphics;
	}
	return renderCommand::commandContext::unknown;
}

template<typename T>
void release(T*& resource)
{
	if (resource != nullptr)
	{
		resource->Release();
		resource = nullptr;
	}
}

bool enableDebugLayer(const bool enableGPUValidation,
	const D3D12_GPU_BASED_VALIDATION_FLAGS gpuBasedValidationFlags,
	const bool enableSynchonizedCommandQueueValidation)
{
	// Enable debug layer
	Microsoft::WRL::ComPtr<ID3D12Debug3> debugInterface;
	if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
	{
		return false;
	}
	debugInterface->EnableDebugLayer();

	// Enable GPU based validation and synchronized command queue validation
	debugInterface->SetEnableGPUBasedValidation(enableGPUValidation);
	debugInterface->SetGPUBasedValidationFlags(gpuBasedValidationFlags);
	debugInterface->SetEnableSynchronizedCommandQueueValidation(enableSynchonizedCommandQueueValidation);

	return true;
}

bool reportLiveObjects()
{
	// Enable reporting of live objects
	Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebugInterface;
	if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebugInterface))))
	{
		return false;
	}
	if (FAILED(dxgiDebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL)))
	{
		return false;
	}
	return true;
}

IDXGIAdapter4* enumerateAdapters(IDXGIFactory7* factory)
{
	IDXGIAdapter4* adapter = nullptr;

	// Get adapters in descending order of performance (highest performance first)
	for (UINT i = 0; factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		// Return the first adapter that supports the minimum feature level
		if (SUCCEEDED(D3D12CreateDevice(adapter, d3d12RenderDevice::minimumSupportedFeatureLevel, __uuidof(ID3D12Device), nullptr)))
		{
			return adapter;
		}

		// Release the adapter as it does not meet minimum requirements
		release(adapter);
	}

	// No adapters were found that meet minimum requirements
	return nullptr;
}

D3D_FEATURE_LEVEL getAdapterMaximumFeatureLevel(IDXGIAdapter4* adapter)
{
	constexpr size_t featureLevelsCount = 5;
	constexpr D3D_FEATURE_LEVEL featureLevels[featureLevelsCount] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_2
	};

	D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo = {};
	featureLevelInfo.NumFeatureLevels = static_cast<UINT>(featureLevelsCount);
	featureLevelInfo.pFeatureLevelsRequested = featureLevels;

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	if (FAILED(D3D12CreateDevice(adapter, d3d12RenderDevice::minimumSupportedFeatureLevel, IID_PPV_ARGS(&device))))
	{
		return D3D_FEATURE_LEVEL();
	}

	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo))))
	{
		return D3D_FEATURE_LEVEL();
	}

	return featureLevelInfo.MaxSupportedFeatureLevel;
}

bool enableDeviceDebugInfo(const Microsoft::WRL::ComPtr<ID3D12Device8>& device)
{
	// Enable device debug info
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
	if (FAILED(device.As(&infoQueue)))
	{
		return false;
	}

	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

	D3D12_MESSAGE_SEVERITY severities[] =
	{
		D3D12_MESSAGE_SEVERITY_INFO
	};

	D3D12_MESSAGE_ID denyIds[] =
	{
		D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
		D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
		D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		// Workarounds for debug layer errors on hybrid-graphics systems
		D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
		D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
	};

	D3D12_INFO_QUEUE_FILTER queueFilter = {};
	queueFilter.DenyList.NumSeverities = _countof(severities);
	queueFilter.DenyList.pSeverityList = severities;
	queueFilter.DenyList.NumIDs = _countof(denyIds);
	queueFilter.DenyList.pIDList = denyIds;

	if (FAILED(infoQueue->PushStorageFilter(&queueFilter)))
	{
		return false;
	}

	return true;
}

bool getTearingSupport(IDXGIFactory7* factory)
{
	BOOL allowTearing = FALSE;

	if (FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
	{
		allowTearing = FALSE;
	}

	return (allowTearing == TRUE);
}

bool waitForValueOnCallingCPUThread(HANDLE fenceEvent, ID3D12Fence* const fence, uint64_t value)
{
	// If the fence's current value is less than the fence value, then the GPU is still
	// executing commands and has not reached the CommandQueue->Signal() command yet
	if (fence->GetCompletedValue() < value)
	{
		// Set event to be executed once the fence value reaches the command frame's fence value
		if (FAILED(fence->SetEventOnCompletion(value, fenceEvent)))
		{
			return false;
		}

		// Wait until the fence has triggered the event 
		WaitForSingleObject(fenceEvent, static_cast<DWORD>(std::chrono::milliseconds::max().count()));
	}

	return true;
}

// ---------------------------------------------
// Descriptor heap
// ---------------------------------------------
bool d3d12DescriptorHeap::init(ID3D12Device8* const device, const UINT descriptorCount, const UINT inDescriptorSize,
	const D3D12_DESCRIPTOR_HEAP_TYPE type, const bool shaderVisible)
{
	descriptorSize = inDescriptorSize;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = descriptorCount;
	desc.Type = type;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))))
	{
		return false;
	}

	LOG(stringHelper::printf("Created descriptor heap of type %s. Allocated descriptor count: %d.\n%s",
		getD3dDescriptorHeapTypeAsString(type).c_str(), descriptorCount, shaderVisible ?
		"Descriptor heap is shader visible." : "Descriptor heap is not shader visible."));
	return true;
}

void d3d12DescriptorHeap::shutdown()
{
	heap.Reset();
}

D3D12_CPU_DESCRIPTOR_HANDLE d3d12DescriptorHeap::getCPUHandleForDescriptorAtHeapStart() const
{
	return heap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE d3d12DescriptorHeap::getCPUHandleForDescriptorAtOffset(UINT offset) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
	handle.ptr = getCPUHandleForDescriptorAtHeapStart().ptr + (offset * descriptorSize);
	return handle;
}

// ---------------------------------------------
// Hardware queue
// ---------------------------------------------
bool d3d12HardwareQueue::init(ID3D12Device8* const device, const D3D12_COMMAND_LIST_TYPE type, 
	const size_t contextSubmissionsPerFrameCount)
{
	// Set hardware queue context
	queueContext = d3d12CommandListTypeToCommandContext(type);

	// Create command queue
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = type;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.NodeMask = 0;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	if (FAILED(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&queue))))
	{
		return false;
	}

	const std::wstring queueName = type == D3D12_COMMAND_LIST_TYPE_DIRECT ? 
		L"graphics_command_queue" : type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? 
		L"compute_command_queue" : L"copy_command_queue";
	if (FAILED(queue->SetName(queueName.c_str())))
	{
		return false;
	}
	LOG(stringHelper::printf("Created %S.", queueName.c_str()));

	// Allocate command list submissions for queue
	commandListSubmissions.resize(static_cast<size_t>(contextSubmissionsPerFrameCount), nullptr);
	LOG(stringHelper::printf("Pre allocated %d render context submissions per frame for %S.", 
		contextSubmissionsPerFrameCount, queueName.c_str()));

	return true;
}

void d3d12HardwareQueue::shutdown()
{
	queue.Reset();
	commandListSubmissions.clear();
}

void d3d12HardwareQueue::submitRenderContexts(const uint32_t numContexts, renderContext*const* contexts)
{
	// Assert that the number of contexts being submitted is the same as the pre allocated amount for the hardware queue
	assert(static_cast<size_t>(numContexts) == commandListSubmissions.size());

	// For each submitted context
	for (uint32_t i = 0; i < numContexts; ++i)
	{
		// Cast to renderer platform context type
		d3d12RenderContext* const d3d12Context = static_cast<d3d12RenderContext* const>(contexts[static_cast<size_t>(i)]);

		// Assert the submitted context command context matches the hardware queue it is being submitted to
		assert(d3d12Context->getCommandContext() == queueContext);

		// Set the pre allocated command list pointer to the submitted context's command list
		commandListSubmissions[static_cast<size_t>(i)] = d3d12Context->getCommandList();
	}

	// Execute the submitted commands on the hardware queue
	queue->ExecuteCommandLists(numContexts, commandListSubmissions.data());
}

// ---------------------------------------------
// Swap chain
// ---------------------------------------------
bool d3d12SwapChain::init(IDXGIFactory7* const factory, ID3D12CommandQueue* const directCommandQueue, 
	ID3D12Device8* const device, const d3d12DescriptorIncrementSizes& descriptorSizes,
	const uint32_t width, const uint32_t height,
	const uint32_t backBufferCount, HWND hwnd)
{
	LOG(stringHelper::printf("Initializing d3d12 swap chain at %dx%d with %d back buffers.", width, height, backBufferCount));

	// Create swap chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = static_cast<UINT>(width);
	swapChainDesc.Height = static_cast<UINT>(height);
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = static_cast<UINT>(backBufferCount);
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = (tearingSupported = getTearingSupport(factory)) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	if (FAILED(factory->CreateSwapChainForHwnd(directCommandQueue,
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1)))
	{
		return false;
	}

	// Disable alt + enter fullscreen shortcut
	if (FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)))
	{
		return false;
	}
	LOG("Disabled alt+enter fullscreen shortcut.");

	// Convert swap chain 1 interface to swap chain 4 interface
	if (FAILED(swapChain1.As(&dxgiSwapChain)))
	{
		return false;
	}

	// Create render target descriptor heap
	const bool rtDescriptorHeapInitResult = rtDescriptorHeap.init(device, static_cast<UINT>(backBufferCount), 
		descriptorSizes.rtv, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);

	if (!rtDescriptorHeapInitResult)
	{
		return false;
	}

	// Create depth stencil descriptor heap
	const bool dsDescriptorHeapInitResult = dsDescriptorHeap.init(device, 1, descriptorSizes.dsv,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false);

	if (!dsDescriptorHeapInitResult)
	{
		return false;
	}

	// Initialize render target view pointers
	rtvs.resize(backBufferCount, nullptr);

	LOG("Initialized d3d12 swap chain.");
	return true;
}

bool d3d12SwapChain::shutdown()
{
	rtDescriptorHeap.shutdown();
	dsDescriptorHeap.shutdown();
	rtvs.clear();
	dsv.Reset();
	dxgiSwapChain.Reset();
	LOG("Shutdown d3d12 swap chain.");
	return true;
}

uint32_t d3d12SwapChain::getCurrentBackBufferIndex() const
{
	return dxgiSwapChain->GetCurrentBackBufferIndex();
}

bool d3d12SwapChain::present(const bool vsyncEnabled) const
{
	return SUCCEEDED(dxgiSwapChain->Present(vsyncEnabled ? 1 : 0,
		(tearingSupported && (!vsyncEnabled)) ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

bool d3d12SwapChain::updateBackBufferRTVs(ID3D12Device8* const device, const UINT rtvDescriptorSize)
{
	// Get a cpu handle to the first rtv descriptor in the rtv heap
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtDescriptorHeap.getCPUHandleForDescriptorAtHeapStart();

	// For each back buffer
	const UINT backBufferCount = static_cast<UINT>(rtvs.size());
	for (UINT i = 0; i < backBufferCount; ++i)
	{
		// Get the buffer from the swap chain
		Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
		if (FAILED(dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer))))
		{
			return false;
		}

		// Create a render target view from the buffer
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

		// Store the back buffer resource
		rtvs[i] = backBuffer;

		// Increment the rtv handle to the next descriptor in the heap
		rtvHandle.ptr += rtvDescriptorSize;
	}

	return true;
}

bool d3d12SwapChain::updateDSV(ID3D12Device8* const device, const UINT64 width, const UINT height)
{
	static constexpr DXGI_FORMAT depthStencilViewFormat = DXGI_FORMAT_D32_FLOAT;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthStencilViewFormat;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = depthStencilViewFormat;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES depthStencilHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC depthStencilResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(depthStencilViewFormat,
		width,
		height,
		1,
		1,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	if (FAILED(device->CreateCommittedResource(&depthStencilHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&dsv)
	)))
	{
		return false;
	}
	if (FAILED(dsv->SetName(L"dsv_resource")))
	{
		return false;
	}

	device->CreateDepthStencilView(dsv.Get(), &dsvDesc, dsDescriptorHeap.getCPUHandleForDescriptorAtHeapStart());

	return true;
}

bool d3d12SwapChain::getDesc(DXGI_SWAP_CHAIN_DESC& outSwapChainDesc) const
{
	return SUCCEEDED(dxgiSwapChain->GetDesc(&outSwapChainDesc));
}

bool d3d12SwapChain::resizeDimensions(ID3D12Device8* const device, const UINT rtvDescriptorSize, 
	const UINT64 width, const UINT height,
	const DXGI_SWAP_CHAIN_DESC& swapChainDesc)
{
	const UINT backBufferCount = static_cast<UINT>(rtvs.size());

	// Release back buffer resources
	rtvs.clear();
	dsv.Reset();

	// Resize swap chain back buffers
	if (FAILED(dxgiSwapChain->ResizeBuffers(backBufferCount,
		static_cast<UINT>(width),
		height,
		swapChainDesc.BufferDesc.Format,
		swapChainDesc.Flags)))
	{
		return false;
	}

	// Recreate rtv descriptors for the swap chain back buffers
	if (!updateBackBufferRTVs(device, rtvDescriptorSize))
	{
		return false;
	}

	// Recreate depth stencil buffer
	if (!updateDSV(device, width, height))
	{
		return false;
	}

	return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE d3d12SwapChain::getCPUBackBufferDescriptorHandle(const uint32_t frameIndex) const
{
	return rtDescriptorHeap.getCPUHandleForDescriptorAtOffset(static_cast<UINT>(frameIndex));
}

D3D12_CPU_DESCRIPTOR_HANDLE d3d12SwapChain::getCPUDepthStencilDescriptorHandle() const
{
	return dsDescriptorHeap.getCPUHandleForDescriptorAtHeapStart();
}

// ---------------------------------------------
// Render context
// ---------------------------------------------
bool d3d12RenderContext::init(ID3D12Device8* const device, const uint8_t inFlightFrameCount, 
	const renderCommand::commandContext commandContext)
{
	setCommandContext(commandContext);

	const D3D12_COMMAND_LIST_TYPE type = commandContextToD3d12CommandListType(commandContext);

	LOG(stringHelper::printf("Initializing d3d12 render context with type: %s and in flight frame count: %d.",
		getCommandListTypeAsString(type).c_str(), inFlightFrameCount));

	// Create command allocators for each in flight frame
	commandAllocators.resize(static_cast<size_t>(inFlightFrameCount), nullptr);
	for (uint32_t i = 0; i < inFlightFrameCount; ++i)
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator = commandAllocators[static_cast<size_t>(i)];

		if (FAILED(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator))))
		{
			return false;
		}

		const std::wstring commandAllocatorName = type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
			std::wstring(L"graphics_command_allocator" + std::to_wstring(i)).c_str() :
			type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
			std::wstring(L"compute_command_allocator" + std::to_wstring(i)).c_str() :
			std::wstring(L"command_allocator" + std::to_wstring(i)).c_str();

		if (FAILED(commandAllocator->SetName(commandAllocatorName.c_str())))
		{
			return false;
		}
		LOG(stringHelper::printf("Created %S.", commandAllocatorName.c_str()));
	}

	// Create command list
	if(FAILED(device->CreateCommandList(0, type, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList))))
	{
		return false;
	}

	if (FAILED(commandList->Close()))
	{
		return false;
	}

	const std::wstring commandListName = type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
		L"graphics_command_list" :
		type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
		L"compute_command_list" : L"command_list";

	if (FAILED(commandList->SetName(commandListName.c_str())))
	{
		return false;
	}
	LOG(stringHelper::printf("Created %S.", commandListName.c_str()));

	return true;
}

bool d3d12RenderContext::shutdown()
{
	commandList.Reset();
	commandAllocators.clear();

	return true;
}

void d3d12RenderContext::submitRenderCommand(const renderCommand& command)
{
	switch (command.getType())
	{
	case renderCommand::commandType::beginContext: renderCommand_beginContext_implementation(static_cast<const renderCommand_beginContext&>(command)); break;
	case renderCommand::commandType::endContext: renderCommand_endContext_implementation(static_cast<const renderCommand_endContext&>(command)); break;
	case renderCommand::commandType::beginFrame: renderCommand_beginFrame_implementation(static_cast<const renderCommand_beginFrame&>(command)); break;
	case renderCommand::commandType::endFrame: renderCommand_endFrame_implementation(static_cast<const renderCommand_endFrame&>(command)); break;
	}
}

void d3d12RenderContext::renderCommand_beginContext_implementation(const renderCommand_beginContext& command)
{
	// Get the command allocator for this back buffer
	ID3D12CommandAllocator* commandAllocator = commandAllocators[static_cast<size_t>(command.frameIndex)].Get();

	// Reset command allocator and list to record new commands
	if (FAILED(commandAllocator->Reset()))
	{
		assert(false);
	}
	if (FAILED(commandList->Reset(commandAllocator, nullptr)))
	{
		assert(false);
	}
}

void d3d12RenderContext::renderCommand_endContext_implementation(const renderCommand_endContext& command)
{
	// Stop recording commands
	if (FAILED(commandList->Close()))
	{
		assert(false);
	}
}

void d3d12RenderContext::renderCommand_beginFrame_implementation(const renderCommand_beginFrame& command)
{
	assert(command.inSwapChain != nullptr);
	d3d12SwapChain* inSwapChain = static_cast<d3d12SwapChain*>(command.inSwapChain);

	// Transition the buffer resource from present state to render target state
	CD3DX12_RESOURCE_BARRIER backBufferResourceTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		inSwapChain->getBackBufferResource(static_cast<size_t>(command.frameIndex)),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &backBufferResourceTransitionBarrier);

	// Get render target resource descriptor for the current back buffer from the heap
	D3D12_CPU_DESCRIPTOR_HANDLE rtDescriptor = inSwapChain->getCPUBackBufferDescriptorHandle(command.frameIndex);

	// Get depth stencil resource descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE dsDescriptor(inSwapChain->getCPUDepthStencilDescriptorHandle());

	// Clear render target
	commandList->ClearRenderTargetView(rtDescriptor, command.clearColorVal, 0, nullptr);

	// Clear depth stencil buffer
	commandList->ClearDepthStencilView(dsDescriptor, 
		command.clearStencil ? D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL :
		D3D12_CLEAR_FLAG_DEPTH,
		command.clearDepthVal, static_cast<UINT8>(command.clearStencilVal), 0, nullptr);

	// Bind render/depth stencil targets to the output merger stage
	commandList->OMSetRenderTargets(1, &rtDescriptor, false, &dsDescriptor);
}

void d3d12RenderContext::renderCommand_endFrame_implementation(const renderCommand_endFrame& command)
{
	assert(command.inSwapChain != nullptr);
	d3d12SwapChain* inSwapChain = static_cast<d3d12SwapChain*>(command.inSwapChain);

	// Transition the buffer resource from render target state to present state
	CD3DX12_RESOURCE_BARRIER backBufferResourceTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		inSwapChain->getBackBufferResource(static_cast<size_t>(command.frameIndex)),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &backBufferResourceTransitionBarrier);
}

// ---------------------------------------------
// Render device
// ---------------------------------------------
bool d3d12RenderDevice::init(const renderDeviceInitSettings& settings)
{
	LOG("Initializing d3d12 render device:");

    if (mainDevice != nullptr)
    {
        shutdown();
    }

#ifdef _DEBUG
	const bool enableDeugLayerResult = 
		enableDebugLayer(false, D3D12_GPU_BASED_VALIDATION_FLAGS_NONE, true);
	if (!enableDeugLayerResult)
	{
		return false;
	}
	LOG("Enabled d3d12 debug layer.");

	const bool reportLiveObjectsResult = reportLiveObjects();
	if (!reportLiveObjectsResult)
	{
		return false;
	}
	LOG("Reporting live objects.")
#endif // _DEBUG

	// Create dxgi factory
	UINT dxgiFactoryCreationFlags = 0;
#ifdef _DEBUG
	dxgiFactoryCreationFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // _DEBUG

	if (FAILED(CreateDXGIFactory2(dxgiFactoryCreationFlags, IID_PPV_ARGS(&dxgiFactory))))
	{
		return false;
	}
	LOG("Created dxgi factory.");

	// Determine adapter to use
	mainAdapter.Attach(enumerateAdapters(dxgiFactory.Get()));

	if (mainAdapter == nullptr)
	{
		return false;
	}

	DXGI_ADAPTER_DESC3 adapterDesc3 = {};
	if (FAILED(mainAdapter->GetDesc3(&adapterDesc3)))
	{
		return false;
	}

	// Check the specified display is connected to the adapter
	if (settings.displayConnectedAdapterName != adapterDesc3.Description)
	{
		return false;
	}
	LOG(stringHelper::printf("Using adapter: %S", adapterDesc3.Description));

	// Check the maximum feature level supported by the adapter is at least the required minimum feature level
	const D3D_FEATURE_LEVEL mainAdapterMaximumSupportedFeatureLevel = 
		getAdapterMaximumFeatureLevel(mainAdapter.Get());

	if (mainAdapterMaximumSupportedFeatureLevel < minimumSupportedFeatureLevel)
	{
		return false;
	}
	LOG("Supported feature level: " + getD3dFeatureLevelAsString(mainAdapterMaximumSupportedFeatureLevel));

	// Create main device
	if (FAILED(D3D12CreateDevice(mainAdapter.Get(), mainAdapterMaximumSupportedFeatureLevel, 
		IID_PPV_ARGS(&mainDevice))))
	{
		return false;
	}

	const std::wstring mainDeviceName = L"main_device";
	if (FAILED(mainDevice->SetName(mainDeviceName.c_str())))
	{
		return false;
	}

#ifdef _DEBUG
	// Enable device debug info
	if (!enableDeviceDebugInfo(mainDevice))
	{
		return false;
	}
#endif // _DEBUG
	LOG(stringHelper::printf("Created %S.", mainDeviceName.c_str()));

	// Retreive descriptor increment sizes
	descriptorSizes.cbv_srv_uav = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizes.rtv = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizes.dsv = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	descriptorSizes.sampler = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	LOG("Retrieved descriptor increment sizes.");

	// Create hardware queues
	const bool graphicsQueueInitResult = graphicsQueue.init(mainDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT,
		settings.graphicsContextSubmissionsPerFrameCount);
	if (!graphicsQueueInitResult)
	{
		return false;
	}

	LOG("Initialized graphics hardware queue.");

	// Create synchronization objects
	currentFenceValue = 0;
	inFlightFenceValues.resize(static_cast<size_t>(settings.buffering == bufferingType::doubleBuffering ?
		2 : 3),
		0);

	if (FAILED(mainDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
	{
		return false;
	}

	const std::wstring fenceName = L"synchronization_fence";
	if (FAILED(fence->SetName(fenceName.c_str())))
	{
		return false;
	}
	LOG(stringHelper::printf("Created %S.", fenceName.c_str()));

	fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

	if (fenceEvent == nullptr)
	{
		return false;
	}
	LOG("Created fence event.");

	LOG("Initialized d3d12 render device.");
    return true;
}

void d3d12RenderDevice::shutdown()
{
	dxgiFactory.Reset();
	mainAdapter.Reset();
	mainDevice.Reset();
	graphicsQueue.shutdown();

	LOG("Shutdown d3d12 render device.");
}

bool d3d12RenderDevice::flush()
{
	// Wait for all in flight frames to finish executing
	const size_t inFlightFrameCount = inFlightFenceValues.size();
	for (size_t i = 0; i < inFlightFrameCount; ++i)
	{
		if (!waitForValueOnCallingCPUThread(fenceEvent, fence.Get(), inFlightFenceValues[i]))
		{
			return false;
		}
	}

	return true;
}

void d3d12RenderDevice::submitRenderContexts(const renderCommand::commandContext commandContext, 
	const uint32_t numContexts, renderContext*const* contexts)
{
	assert(commandContext != renderCommand::commandContext::unknown);

	switch (commandContext)
	{
		case renderCommand::commandContext::graphics:
		{
			graphicsQueue.submitRenderContexts(numContexts, contexts);
		}
		break;
	}
}

bool d3d12RenderDevice::createRenderContext(const renderCommand::commandContext commandContext,
	std::unique_ptr<renderContext>& outRenderContext) const
{
	d3d12RenderContext* newContext = new d3d12RenderContext;

	const bool initResult = newContext->init(mainDevice.Get(),
		static_cast<uint8_t>(inFlightFenceValues.size()), commandContext);

	if (!initResult)
	{
		return false;
	}

	outRenderContext.reset(newContext);

	return true;
}

bool d3d12RenderDevice::destroyRenderContext(std::unique_ptr<renderContext>& outRenderContext)
{
	d3d12RenderContext* d3d12Context = static_cast<d3d12RenderContext*>(outRenderContext.get());
	const bool shutdownResult = d3d12Context->shutdown();
	if (!shutdownResult)
	{
		return false;
	}
	outRenderContext.reset();
	return true;
}

bool d3d12RenderDevice::createSwapChain(const swapChainInitSettings& settings,
	std::unique_ptr<swapChain>& outSwapChain)
{
	d3d12SwapChain* newSwapChain = new d3d12SwapChain;

	const bool initResult = newSwapChain->init(dxgiFactory.Get(), graphicsQueue.GetCommandQueue(), 
		mainDevice.Get(), descriptorSizes,
		settings.width, settings.height, static_cast<uint32_t>(inFlightFenceValues.size()), 
		static_cast<HWND>(settings.windowHandle));

	if (!initResult)
	{
		return false;
	}

	const bool updateRTVResult = newSwapChain->updateBackBufferRTVs(mainDevice.Get(), descriptorSizes.rtv);
	if (!updateRTVResult)
	{
		return false;
	}

	const bool updateDSVResult = newSwapChain->updateDSV(mainDevice.Get(), settings.width, settings.height);
	if (!updateDSVResult)
	{
		return false;
	}

	outSwapChain.reset(newSwapChain);
	return true;
}

bool d3d12RenderDevice::destroySwapChain(std::unique_ptr<swapChain>& outSwapChain)
{
	assert(outSwapChain != nullptr);
	d3d12SwapChain* inD3d12SwapChain = static_cast<d3d12SwapChain*>(outSwapChain.get());
	const bool shutdownResult = inD3d12SwapChain->shutdown();
	if (!shutdownResult)
	{
		return false;
	}
	outSwapChain.reset();
	return true;
}

bool d3d12RenderDevice::resizeSwapChainDimensions(swapChain* inSwapChain, const uint32_t newWidth, const uint32_t newHeight)
{
	assert(inSwapChain != nullptr);
	d3d12SwapChain* inD3d12SwapChain = static_cast<d3d12SwapChain*>(inSwapChain);

	// Get swap chain description
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	const bool getSwapChainDescResult = inD3d12SwapChain->getDesc(swapChainDesc);
	if (!getSwapChainDescResult)
	{
		return false;
	}

	// Check if the new dimensions are the same as the current swap chain dimensions
	if ((swapChainDesc.BufferDesc.Width == static_cast<UINT>(newWidth)) &&
		(swapChainDesc.BufferDesc.Height == static_cast<UINT>(newHeight)))
	{
		// Return without resizing the swap chain back buffers
		return true;
	}

	// Flush command queues
	const bool flushResult = flush();
	if (!flushResult)
	{
		return false;
	}

	// Reset in flight fence value to current fence value
	std::fill(inFlightFenceValues.begin(), inFlightFenceValues.end(), currentFenceValue);

	// Resize swap chain
	const bool swapChainResizeResult = inD3d12SwapChain->resizeDimensions(mainDevice.Get(),
		descriptorSizes.rtv, static_cast<UINT64>(newWidth), static_cast<UINT>(newHeight), swapChainDesc);
	if (!swapChainResizeResult)
	{
		return false;
	}

	LOG(stringHelper::printf("Resized swap chain with new dimensions %dx%d.", newWidth, newHeight));
	return true;
}

bool d3d12RenderDevice::synchronizeBeginFrame(uint32_t inCurrentFrameIndex)
{
	return waitForValueOnCallingCPUThread(fenceEvent, fence.Get(), inFlightFenceValues[static_cast<size_t>(inCurrentFrameIndex)]);
}

bool d3d12RenderDevice::synchronizeEndFrame(uint32_t inCurrentFrameIndex)
{
	// Increment the fence value and set the current back buffer's fence value to this
	inFlightFenceValues[static_cast<size_t>(inCurrentFrameIndex)] = ++currentFenceValue;

	// Insert command at the end of queues to signal the fence with the current context fence value to indicate 
	// that the GPU has finished executing the commands submitted during render context submission
	if (FAILED(graphicsQueue.GetCommandQueue()->Signal(fence.Get(), currentFenceValue)))
	{
		return false;
	}

	return true;
}
