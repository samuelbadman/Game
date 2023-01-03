#include "d3d12Renderer.h"
#include "log.h"
#include "stringHelper.h"
#include "platform/win32/win32Display.h"

#include <cassert>

bool d3d12Renderer::init(const rendererInitSettings& settings)
{
	LOG("Initializing d3d12 renderer.");

    if (mainDevice != nullptr)
    {
        shutdown();
    }

#ifdef _DEBUG
	const bool enableDeugLayerResult = 
		enableDebugLayer(false, D3D12_GPU_BASED_VALIDATION_FLAGS_NONE, false);
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
	if (win32Display::infoForDisplayAtIndex(settings.displayIndex).adapterName != adapterDesc3.Description)
	{
		return false;
	}
	LOG(stringHelper::printf("Using adapter: %S", adapterDesc3.Description));



	LOG("Initialized d3d12 renderer.");
    return true;
}

bool d3d12Renderer::shutdown()
{
	dxgiFactory.Reset();
	mainAdapter.Reset();

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
		if (SUCCEEDED(D3D12CreateDevice(adapter, minSupportedFeatureLevel, __uuidof(ID3D12Device), nullptr)))
		{
			return adapter;
		}

		// Release the adapter as it does not meet minimum requirements
		release(adapter);
	}

	// No adapters were found that meet minimum requirements
	return nullptr;
}
