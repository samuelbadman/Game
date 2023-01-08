#include "pch.h"
#include "d3d12Renderer.h"
#include "log.h"
#include "stringHelper.h"

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

bool waitForValueOnCurrentCPUThread(HANDLE fenceEvent, ID3D12Fence* const fence, uint64_t value)
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
	LOG(stringHelper::printf("Context submissions per frame allocated: %d", contextSubmissionsPerFrameCount));

	return true;
}

void d3d12HardwareQueue::shutdown()
{
	queue.Reset();
	commandListSubmissions.clear();
}

void d3d12HardwareQueue::submitRenderContexts(const uint32_t numContexts, renderContext*const* contexts)
{
	assert(static_cast<size_t>(numContexts) == commandListSubmissions.size());

	for (uint32_t i = 0; i < numContexts; ++i)
	{
		d3d12RenderContext* const d3d12Context = static_cast<d3d12RenderContext* const>(contexts[static_cast<size_t>(i)]);

		assert(d3d12Context->getCommandContext() == queueContext);

		commandListSubmissions[static_cast<size_t>(i)] = d3d12Context->getCommandList();
	}

	queue->ExecuteCommandLists(numContexts, commandListSubmissions.data());
}

bool d3d12RenderContext::init(ID3D12Device8* const device, const uint8_t inFlightFrameCount, 
	const renderCommand::commandContext commandContext)
{
	setCommandContext(commandContext);

	const D3D12_COMMAND_LIST_TYPE type = commandContextToD3d12CommandListType(commandContext);

	LOG(stringHelper::printf("Initializing render context with type: %s and in flight frame count: %d.",
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
}

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
		if (!waitForValueOnCurrentCPUThread(fenceEvent, fence.Get(), inFlightFenceValues[i]))
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
	assert(d3d12Context != nullptr);
	const bool shutdownResult = d3d12Context->shutdown();
	if (!shutdownResult)
	{
		return false;
	}
	outRenderContext.reset();
	return true;
}
