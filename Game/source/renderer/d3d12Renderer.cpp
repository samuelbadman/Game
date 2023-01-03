#include "pch.h"
#include "d3d12Renderer.h"
#include "log.h"
#include "stringHelper.h"
#include "platform/win32/win32Display.h"

bool d3d12Renderer::init(const rendererInitSettings& settings)
{
	LOG("Initializing d3d12 renderer.");

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
	const displayInfo display = win32Display::infoForDisplayAtIndex(settings.displayIndex);
	if (display.adapterName != adapterDesc3.Description)
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

	if (FAILED(mainDevice->SetName(L"main_device")))
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
	LOG("Created d3d12 device.");

	// Retreive descriptor increment sizes
	descriptorSizes.cbv_srv_uav = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizes.rtv = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizes.dsv = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	descriptorSizes.sampler = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	LOG("Retrieved descriptor increment sizes.");



	LOG("Initialized d3d12 renderer.");
    return true;
}

bool d3d12Renderer::shutdown()
{
	dxgiFactory.Reset();
	mainAdapter.Reset();
	mainDevice.Reset();

	LOG("Shutdown d3d12 renderer.");
	return true;
}

bool d3d12Renderer::enableDebugLayer(const bool enableGPUValidation,
	const D3D12_GPU_BASED_VALIDATION_FLAGS gpuBasedValidationFlags,
	const bool enableSynchonizedCommandQueueValidation) const
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

bool d3d12Renderer::reportLiveObjects() const
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

IDXGIAdapter4* d3d12Renderer::enumerateAdapters(IDXGIFactory7* factory) const
{
	IDXGIAdapter4* adapter = nullptr;

	// Get adapters in descending order of performance (highest performance first)
	for (UINT i = 0; factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		// Return the first adapter that supports the minimum feature level
		if (SUCCEEDED(D3D12CreateDevice(adapter, minimumSupportedFeatureLevel, __uuidof(ID3D12Device), nullptr)))
		{
			return adapter;
		}

		// Release the adapter as it does not meet minimum requirements
		release(adapter);
	}

	// No adapters were found that meet minimum requirements
	return nullptr;
}

D3D_FEATURE_LEVEL d3d12Renderer::getAdapterMaximumFeatureLevel(IDXGIAdapter4* adapter) const
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
	if (FAILED(D3D12CreateDevice(adapter, minimumSupportedFeatureLevel, IID_PPV_ARGS(&device))))
	{
		return D3D_FEATURE_LEVEL();
	}

	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo))))
	{
		return D3D_FEATURE_LEVEL();
	}

	return featureLevelInfo.MaxSupportedFeatureLevel;
}

bool d3d12Renderer::enableDeviceDebugInfo(const Microsoft::WRL::ComPtr<ID3D12Device8>& device) const
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

std::string d3d12Renderer::getD3dFeatureLevelAsString(const D3D_FEATURE_LEVEL featureLevel) const
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

bool d3d12Renderer::getTearingSupport(IDXGIFactory7* factory) const
{
	BOOL allowTearing = FALSE;

	if (FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
	{
		allowTearing = FALSE;
	}

	return (allowTearing == TRUE);
}
