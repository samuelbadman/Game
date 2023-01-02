#include "d3d12Renderer.h"

bool d3d12Renderer::init()
{
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

	const bool reportLiveObjectsResult = dxgiReportLiveObjects();
	if (!reportLiveObjectsResult)
	{
		return false;
	}
#endif // _DEBUG



    return false;
}

bool d3d12Renderer::shutdown()
{
    return false;
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

bool d3d12Renderer::dxgiReportLiveObjects() const
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
