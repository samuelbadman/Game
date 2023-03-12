#include "pch.h"
#include "direct3d12Graphics.h"
#include "platform/framework/abstract/platformMessageBox.h"
#include "direct3d12Surface.h"
#include "fileIO/fileIO.h"
#include "platform/graphics/sVertexPos3Norm3Col4UV2.h"
#include "platform/graphics/sMeshResources.h"
#include "math/matrix4x4.h"
#include "platform/graphics/sRenderData.h"

using namespace Microsoft::WRL;

#define fatalIfFailed(x) if(FAILED(x)) platformLayer::messageBox::showMessageBoxFatal("hresult failed.");

static constexpr size_t kb_64 = 65536;
static constexpr size_t constantBufferDataStepSize = 256;
static constexpr DWORD maxFenceWaitDurationMs = static_cast<DWORD>(std::chrono::milliseconds::max().count());

struct sObjectConstantBuffer
{
	float worldViewProjectionMatrix[16];
};

struct sCameraConstantBuffer
{
};

static void enableDebugLayer()
{
#if defined(_DEBUG)
	// Enable the d3d12 debug layer
	ComPtr<ID3D12Debug> debugInterface;
	fatalIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif // defined(_DEBUG)
}

static void createDxgiFactory(ComPtr<IDXGIFactory7>& outFactory)
{
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // defined(_DEBUG)

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
		// Get adapters in descending order of performance. Highest performance adapter is the first loop iteration.
		for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&outAdapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			// Select the first adapter that supports the minimum feature level.
			if (SUCCEEDED(D3D12CreateDevice(outAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
			{
				return;
			}
		}

		// No adapters were found that meet minimum requirements
		platformLayer::messageBox::showMessageBoxFatal("direct3d12Graphics: did not find any suitable adapter");
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
		platformLayer::messageBox::showMessageBoxFatal("direct3d12Graphics::createDevice: failed to cast device to info queue.Could not enable device debug info.");
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
		platformLayer::messageBox::showMessageBoxFatal("direct3d12Graphics::createDevice: failed to push queue filter. Could not enable device debug info.");
	}
#endif // defined(_DEBUG)
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
		rtvHandle.ptr += rtvDescriptorSize;
	}
}

static void updateDepthStencilView(ID3D12Device8* device, 
	uint32_t width, 
	uint32_t height, 
	ComPtr<ID3D12Resource>& depthStencilView, 
	ID3D12DescriptorHeap* dsvDescriptorHeap)
{
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc = { 1, 0 };
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	fatalIfFailed(device->CreateCommittedResource(&heapProperties, 
		D3D12_HEAP_FLAG_NONE, 
		&resourceDesc, 
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue, 
		IID_PPV_ARGS(&depthStencilView)));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(depthStencilView.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
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
		platformLayer::messageBox::showMessageBoxFatal("direct3d12Graphics::CreateEvent: failed to create event handle.");
	}
}

static void waitForFence(ID3D12Fence* fence, HANDLE inEventHandle, uint64_t value, DWORD waitDuration = maxFenceWaitDurationMs)
{
	if (fence->GetCompletedValue() < value)
	{
		fatalIfFailed(fence->SetEventOnCompletion(value, inEventHandle));
		if (WaitForSingleObject(inEventHandle, waitDuration) == WAIT_FAILED)
		{
			platformLayer::messageBox::showMessageBoxFatal("direct3d12Graphics::waitForFence: failed to wait for single object.");
		}
	}
}

void createDxcLibrary(ComPtr<IDxcLibrary>& outDxcLibrary)
{
	// Create Dxc library instance
	fatalIfFailed(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&outDxcLibrary)));
}

void createDxcCompiler(ComPtr<IDxcCompiler>& outDxcCompiler)
{
	// Create Dxc compiler instance
	fatalIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&outDxcCompiler)));
}

// Returns non-zero if the shader compiles succesfully
uint8_t compileShaderFromFile(IDxcLibrary* dxcLibrary,
	IDxcCompiler* dxcCompiler,
	LPCWSTR file,
	LPCWSTR entryPoint,
	LPCWSTR targetProfile,
	ComPtr<IDxcBlob>& outBlob,
	std::string& outErrorMessage)
{
	uint32_t codePage = CP_UTF8;
	ComPtr<IDxcBlobEncoding> sourceBlob;
	fatalIfFailed(dxcLibrary->CreateBlobFromFile(file, &codePage, &sourceBlob));

	ComPtr<IDxcOperationResult> result;
	fatalIfFailed(dxcCompiler->Compile(sourceBlob.Get(), file, entryPoint, targetProfile, nullptr, 0, nullptr, 0, nullptr, &result));

	HRESULT hr;
	fatalIfFailed(result->GetStatus(&hr));
	if (FAILED(hr))
	{
		ComPtr<IDxcBlobEncoding> errorBlob;
		fatalIfFailed(result->GetErrorBuffer(&errorBlob));
		if (errorBlob != nullptr)
		{
			outErrorMessage = static_cast<const char*>(errorBlob->GetBufferPointer());
			return 0;
		}
	}

	fatalIfFailed(result->GetResult(&outBlob));
	return 1;
}

static void createCommittedBuffer(ID3D12Device8* device, D3D12_HEAP_TYPE type, UINT64 width, D3D12_RESOURCE_STATES initialResourceState, ComPtr<ID3D12Resource>& outResource)
{
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(type);
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(width);
	fatalIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialResourceState, nullptr, IID_PPV_ARGS(&outResource)));
}

static void updateBufferResource(ID3D12Resource* buffer, const void* src, const size_t width)
{
	D3D12_RANGE readRange = {};
	void* resourceData;
	fatalIfFailed(buffer->Map(0, &readRange, &resourceData));
	memcpy(resourceData, src, width);
	buffer->Unmap(0, nullptr);
}

void direct3d12ConstantBuffer::init(ID3D12Device8* device, const UINT64 inHeapWidth, const size_t constantDataWidth)
{
	assert((inHeapWidth % kb_64) == 0); // Constant buffers must be allocated with a width that is a multiple of 64 kilobytes (65536 bytes)
	createCommittedBuffer(device, D3D12_HEAP_TYPE_UPLOAD, inHeapWidth, D3D12_RESOURCE_STATE_GENERIC_READ, buffer);
	heapWidth = inHeapWidth;
	D3D12_RANGE readRange = {};
	fatalIfFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&bufferStartPointer)));
	dataStepRate = static_cast<uint32_t>(((constantDataWidth % constantBufferDataStepSize) == 0) ? 1 : (constantDataWidth / constantBufferDataStepSize) + 1); // Integer division ignores remainder
}

void direct3d12ConstantBuffer::shutdown()
{
	buffer->Unmap(0, nullptr);
	buffer.Reset();
	*this = {};
}

void direct3d12ConstantBuffer::update(const void* const src, const size_t size)
{
	memcpy(bufferStartPointer + getOffsetToCurrentPosition(), src, size);
}

D3D12_GPU_VIRTUAL_ADDRESS direct3d12ConstantBuffer::GetGPUVirtualAddress()
{
	return buffer->GetGPUVirtualAddress() + getOffsetToCurrentPosition();
}

void direct3d12ConstantBuffer::increment()
{
	bufferPositionIndex = (bufferPositionIndex + 1) % (heapWidth / constantBufferDataStepSize);
}

uint32_t direct3d12ConstantBuffer::getOffsetToCurrentPosition()
{
	return (bufferPositionIndex * constantBufferDataStepSize);
}

direct3d12Graphics::direct3d12Graphics()
	: graphics(eGraphicsApi::direct3d12)
{
}

void direct3d12Graphics::init(bool useWarp, uint32_t inBackBufferCount)
{
	waitForGPU();

	backBufferCount = inBackBufferCount;

	enableDebugLayer();
	createDxgiFactory(dxgiFactory);
	getAdapter(dxgiFactory.Get(), useWarp, adapter);
	createDevice(adapter.Get(), device);
	descriptorSizes = getDescriptorSizes(device.Get());
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, graphicsQueue);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE, computeQueue);
	createCommandQueue(device.Get(), D3D12_COMMAND_LIST_TYPE_COPY, copyQueue);

	graphicsCommandAllocators.resize(static_cast<size_t>(backBufferCount));
	for (ComPtr<ID3D12CommandAllocator>& commandAllocator : graphicsCommandAllocators)
	{
		static uint32_t index = 0;
		createCommandAllocator(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator);
		commandAllocator->SetName((std::wstring(L"graphicsCommandAllocator") + std::to_wstring(index)).c_str());
		++index;
	}
	createCommandList(device.Get(), graphicsCommandAllocators[0].Get(), nullptr, D3D12_COMMAND_LIST_TYPE_DIRECT, graphicsCommandList);

	createFence(device.Get(), graphicsFence);
	graphicsFenceValue = 0;
	graphicsFenceValues.resize(static_cast<size_t>(backBufferCount));
	std::fill(graphicsFenceValues.begin(), graphicsFenceValues.end(), graphicsFenceValue);

	createEventHandle(eventHandle);

	createDxcLibrary(dxcLibrary);
	createDxcCompiler(dxcCompiler);

	// Create constant buffer
	objectConstantBuffer.init(device.Get(), kb_64, sizeof(objectConstantBuffer));
	cameraConstantBuffer.init(device.Get(), kb_64, sizeof(cameraConstantBuffer));

	// Input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.NumElements = _countof(inputLayout);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	// Root signature
	D3D12_ROOT_DESCRIPTOR objectRootCBDescriptor = {};
	objectRootCBDescriptor.ShaderRegister = 0;
	objectRootCBDescriptor.RegisterSpace = 0;

	D3D12_ROOT_DESCRIPTOR cameraRootCBDescriptor = {};
	cameraRootCBDescriptor.ShaderRegister = 1;
	cameraRootCBDescriptor.RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameters[2];

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor = objectRootCBDescriptor;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor = cameraRootCBDescriptor;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;

	ID3DBlob* signature;
	fatalIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, nullptr));
	fatalIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

	// Shader compilation
	std::vector<uint8_t> vertexShaderBuffer;
	loadShader("shaders/direct3d12/vertexShader.hlsl", L"main", L"vs_6_0", vertexShaderBuffer);

	std::vector<uint8_t> pixelShaderBuffer;
	loadShader("shaders/direct3d12/pixelShader.hlsl", L"main", L"ps_6_0", pixelShaderBuffer);

	// Pipeline state
	CD3DX12_DEFAULT def = {};

	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(def);
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE,
		FALSE /* FrontCounterClockwise */,
		D3D12_DEFAULT_DEPTH_BIAS,
		D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE /* DepthClipEnable */,
		TRUE /* MultisampleEnable */,
		FALSE /* AntialiasedLineEnable */,
		0 /* ForceSampleCount */,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

	CD3DX12_BLEND_DESC blendDesc(def);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.VS.BytecodeLength = vertexShaderBuffer.size();
	graphicsPipelineStateDesc.VS.pShaderBytecode = vertexShaderBuffer.data();
	graphicsPipelineStateDesc.PS.BytecodeLength = pixelShaderBuffer.size();
	graphicsPipelineStateDesc.PS.pShaderBytecode = pixelShaderBuffer.data();
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.SampleDesc = { 1,0 };
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.NumRenderTargets = 1;

	fatalIfFailed(device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState)));
}

void direct3d12Graphics::shutdown()
{
	waitForGPU();

	objectConstantBuffer.shutdown();
	cameraConstantBuffer.shutdown();
	resourceStore.clear();
	vertexBufferViewStore.clear();
	indexBufferViewStore.clear();

	backBufferCount = 0;
	dxgiFactory.Reset();
	adapter.Reset();
	device.Reset();
	graphicsQueue.Reset();
	computeQueue.Reset();
	copyQueue.Reset();
	descriptorSizes = {};
	graphicsCommandAllocators.clear();
	graphicsCommandList.Reset();
	graphicsFence.Reset();
	graphicsFenceValue = 0;
	graphicsFenceValues.clear();
	CloseHandle(eventHandle);
	currentFrameIndex = 0;

	dxcLibrary.Reset();
	dxcCompiler.Reset();

	rootSignature.Reset();
	graphicsPipelineState.Reset();

}

void direct3d12Graphics::createSurface(void* hwnd, uint32_t width, uint32_t height, bool vsync, std::shared_ptr<graphicsSurface>& outSurface)
{
	outSurface = std::make_shared<direct3d12Surface>();
	direct3d12Surface* const apiSurface = outSurface->as<direct3d12Surface>();

	createSwapChain(dxgiFactory.Get(), graphicsQueue.Get(), static_cast<HWND>(hwnd), width, height, backBufferCount, apiSurface->swapChain);
	apiSurface->currentBackBufferIndex = apiSurface->swapChain->GetCurrentBackBufferIndex();
	createDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, backBufferCount, false, apiSurface->rtvDescriptorHeap);
	apiSurface->renderTargetViews.resize(static_cast<size_t>(backBufferCount));
	updateRenderTargetViews(device.Get(),
		apiSurface->swapChain.Get(),
		descriptorSizes.rtvDescriptorSize, 
		apiSurface->renderTargetViews,
		apiSurface->rtvDescriptorHeap.Get(),
		backBufferCount);
	createDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false, apiSurface->dsvDescriptorHeap);
	updateDepthStencilView(device.Get(), width, height, apiSurface->depthStencilView, apiSurface->dsvDescriptorHeap.Get());

	// Surfaces currently only support a single viewport. There is currently no support for splitscreen
	D3D12_VIEWPORT& outViewport = apiSurface->viewport;
	outViewport.Width = static_cast<FLOAT>(width);
	outViewport.Height = static_cast<FLOAT>(height);
	outViewport.TopLeftX = 0.0f;
	outViewport.TopLeftY = 0.0f;
	outViewport.MinDepth = 0.0f;
	outViewport.MaxDepth = 1.0f;
	D3D12_RECT& outScissorRect = apiSurface->scissorRect;
	outScissorRect.top = 0;
	outScissorRect.left = 0;
	outScissorRect.right = width;
	outScissorRect.bottom = height;

	apiSurface->useVSync = vsync;

	createFence(device.Get(), apiSurface->resourceFence);
	apiSurface->resourceFenceValue = 0;
	apiSurface->resourceFenceValues.resize(backBufferCount, apiSurface->resourceFenceValue);
}

void direct3d12Graphics::destroySurface(std::shared_ptr<graphicsSurface>& surface)
{
	assert(surface->getApi() == eGraphicsApi::direct3d12);

	waitForGPU();

	direct3d12Surface* const apiSurface = surface->as<direct3d12Surface>();

	apiSurface->swapChain.Reset();
	apiSurface->renderTargetViews.clear();
	apiSurface->rtvDescriptorHeap.Reset();
	apiSurface->depthStencilView.Reset();
	apiSurface->dsvDescriptorHeap.Reset();
	apiSurface->resourceFence.Reset();
	apiSurface->resourceFenceValues.clear();

	surface.reset();
}

void direct3d12Graphics::resizeSurface(graphicsSurface* surface, uint32_t width, uint32_t height)
{
	assert(surface->getApi() == eGraphicsApi::direct3d12);

	direct3d12Surface* const apiSurface = surface->as<direct3d12Surface>();

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	fatalIfFailed(apiSurface->swapChain->GetDesc(&swapChainDesc));

	// Don't resize if the width and height have not changed
	if (swapChainDesc.BufferDesc.Width != width || swapChainDesc.BufferDesc.Height != height)
	{
		// Don't allow 0 size swap chain back buffers
		width = std::max(1u, width);
		height = std::max(1u, height);

		// Wait for surface resources to be finished with on the GPU
		waitForFence(apiSurface->resourceFence.Get(), eventHandle, apiSurface->resourceFenceValue);

		// Release render target view resources
		const size_t backBufferCount = apiSurface->renderTargetViews.size();
		for (size_t i = 0; i < backBufferCount; ++i)
		{
			apiSurface->renderTargetViews[i].Reset();
			apiSurface->resourceFenceValues[i] = apiSurface->resourceFenceValue;
		}

		// Resize the swap chain back buffers
		fatalIfFailed(apiSurface->swapChain->ResizeBuffers(static_cast<UINT>(backBufferCount), width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		// Update current back buffer index
		apiSurface->currentBackBufferIndex = apiSurface->swapChain->GetCurrentBackBufferIndex();

		// Update render target view resources with new back buffers
		updateRenderTargetViews(device.Get(),
			apiSurface->swapChain.Get(),
			descriptorSizes.rtvDescriptorSize, 
			apiSurface->renderTargetViews,
			apiSurface->rtvDescriptorHeap.Get(),
			static_cast<UINT>(backBufferCount));

		// Update depth stencil view resource
		updateDepthStencilView(device.Get(), width, height, apiSurface->depthStencilView, apiSurface->dsvDescriptorHeap.Get());

		// Update viewport
		D3D12_VIEWPORT& outViewport = apiSurface->viewport;
		outViewport.Width = static_cast<FLOAT>(width);
		outViewport.Height = static_cast<FLOAT>(height);

		// Update scissor rect
		D3D12_RECT& outScissorRect = apiSurface->scissorRect;
		outScissorRect.right = width;
		outScissorRect.bottom = height;
	}
}

void direct3d12Graphics::setSurfaceUseVSync(graphicsSurface* surface, const bool inUseVSync)
{
	assert(surface->getApi() == eGraphicsApi::direct3d12);
	direct3d12Surface* const apiSurface = surface->as<direct3d12Surface>();
	apiSurface->useVSync = static_cast<BOOL>(inUseVSync);
}

void direct3d12Graphics::beginFrame()
{
	// Wait for the previous frame to finish on the GPU
	waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[currentFrameIndex]);

	// Get frame resources
	ID3D12CommandAllocator* const graphicsCommandAllocator = graphicsCommandAllocators[currentFrameIndex].Get();

	// Start recording command list
	fatalIfFailed(graphicsCommandAllocator->Reset());
	fatalIfFailed(graphicsCommandList->Reset(graphicsCommandAllocator, nullptr));
}

void direct3d12Graphics::render(const uint32_t numSurfaces, class graphicsSurface* const* surfaces, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const matrix4x4* const viewProjection)
{
	// For each surface
	for(uint32_t i = 0; i < numSurfaces; ++i)
	{
		graphicsSurface* const surface = surfaces[static_cast<size_t>(i)];
		assert(surface->getApi() == eGraphicsApi::direct3d12);
		direct3d12Surface* const apiSurface = surface->as<direct3d12Surface>();

		recordSurface(apiSurface, graphicsCommandList.Get(), renderDataCount, renderData, viewProjection);
	}
}

void direct3d12Graphics::endFrame(const uint32_t numRenderedSurfaces, graphicsSurface* const* renderedSurfaces)
{
	// Stop recording command list
	fatalIfFailed(graphicsCommandList->Close());

	// Execute command lists
	ID3D12CommandList* graphicsExecuteLists[] = { graphicsCommandList.Get() };
	graphicsQueue->ExecuteCommandLists(_countof(graphicsExecuteLists), graphicsExecuteLists);

	// Present each surface
	static bool tearingSupported = checkTearingSupport(dxgiFactory.Get());
	for (uint32_t i = 0; i < numRenderedSurfaces; ++i)
	{
		// Get surface as direct3d12 surface
		graphicsSurface* const surface = renderedSurfaces[static_cast<size_t>(i)];
		assert(surface->getApi() == eGraphicsApi::direct3d12);
		direct3d12Surface* apiSurface = renderedSurfaces[static_cast<size_t>(i)]->as<direct3d12Surface>();
		
		// Insert signal after command submission and wait for surface to render before presenting
		++apiSurface->resourceFenceValue;
		apiSurface->resourceFenceValues[apiSurface->currentBackBufferIndex] = apiSurface->resourceFenceValue;
		fatalIfFailed(graphicsQueue->Signal(apiSurface->resourceFence.Get(), apiSurface->resourceFenceValues[apiSurface->currentBackBufferIndex]));

		waitForFence(apiSurface->resourceFence.Get(), eventHandle, apiSurface->resourceFenceValue);

		// Present surface and update back buffer index
		presentSurface(apiSurface, apiSurface->useVSync, tearingSupported);
		apiSurface->currentBackBufferIndex = apiSurface->swapChain->GetCurrentBackBufferIndex();
	}

	// Signal end frame. Must be done after present as flip discard swap effect is being used and this presents without blocking CPU thread
	++graphicsFenceValue;
	graphicsFenceValues[currentFrameIndex] = graphicsFenceValue;
	fatalIfFailed(graphicsQueue->Signal(graphicsFence.Get(), graphicsFenceValues[currentFrameIndex]));
	currentFrameIndex = (currentFrameIndex + 1) % backBufferCount;
}

//void direct3d12Graphics::loadMesh(const size_t vertexCount, const sVertexPos3Norm3Col4UV2* const vertices, const size_t indexCount, const uint32_t* const indices, sMeshResources& outMeshResource)
//{
//	const size_t vertexBufferWidth = sizeof(sVertexPos3Norm3Col4UV2) * vertexCount;
//	const size_t indexBufferWidth = sizeof(uint32_t) * indexCount;
//
//	outMeshResource.indexCount = static_cast<uint32_t>(indexCount);
//
//	// Create and write to upload heaps
//	ComPtr<ID3D12Resource> vertexUploadBuffer;
//	createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_UPLOAD, vertexBufferWidth, D3D12_RESOURCE_STATE_GENERIC_READ, vertexUploadBuffer);
//	updateBufferResource(vertexUploadBuffer.Get(), vertices, vertexBufferWidth);
//
//	ComPtr<ID3D12Resource> indexUploadBuffer;
//	createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_UPLOAD, indexBufferWidth, D3D12_RESOURCE_STATE_GENERIC_READ, indexUploadBuffer);
//	updateBufferResource(indexUploadBuffer.Get(), indices, indexBufferWidth);
//
//	// Create default heaps and submit copy upload heaps to default heaps work to the graphics queue
//	// Before the CPU can reset the graphics command allocator for the current frame, it needs to wait until previous work stored in the allocator has completed on the GPU
//	waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[currentFrameIndex], maxFenceWaitDurationMs);
//	ID3D12CommandAllocator* graphicsCommandAllocator = graphicsCommandAllocators[currentFrameIndex].Get();
//	fatalIfFailed(graphicsCommandAllocator->Reset());
//	fatalIfFailed(graphicsCommandList->Reset(graphicsCommandAllocator, nullptr));
//
//	createDefaultBufferAndRecordCopyCommand(graphicsCommandList.Get(), vertexUploadBuffer.Get(), static_cast<UINT>(vertexBufferWidth), outMeshResource.vertexBufferResourceHandle);
//	createDefaultBufferAndRecordCopyCommand(graphicsCommandList.Get(), indexUploadBuffer.Get(), static_cast<UINT>(indexBufferWidth), outMeshResource.indexBufferResourceHandle);
//
//	D3D12_RESOURCE_BARRIER resourceBarriers[] = {
//		CD3DX12_RESOURCE_BARRIER::Transition(resourceStore[outMeshResource.vertexBufferResourceHandle].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
//		CD3DX12_RESOURCE_BARRIER::Transition(resourceStore[outMeshResource.indexBufferResourceHandle].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)
//	};
//	graphicsCommandList->ResourceBarrier(_countof(resourceBarriers), resourceBarriers);
//
//	fatalIfFailed(graphicsCommandList->Close());
//	ID3D12CommandList* commandLists[] = { graphicsCommandList.Get() };
//	graphicsQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
//
//	graphicsFenceValues[currentFrameIndex] = ++graphicsFenceValue;
//	graphicsQueue->Signal(graphicsFence.Get(), graphicsFenceValue);
//
//	// Create buffer views
//	createVertexBufferView(outMeshResource.vertexBufferResourceHandle, sizeof(sVertexPos3Norm3Col4UV2), static_cast<UINT>(vertexBufferWidth), outMeshResource.vertexBufferViewHandle);
//	createIndexBufferView(outMeshResource.indexBufferResourceHandle, DXGI_FORMAT_R32_UINT, static_cast<UINT>(indexBufferWidth), outMeshResource.indexBufferViewHandle);
//
//	// Wait for the copy work to finish as the upload buffers are released after exiting this function
//	waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[currentFrameIndex], maxFenceWaitDurationMs);
//}

void direct3d12Graphics::loadMeshes(const uint32_t meshCount, const size_t* vertexCounts, const sVertexPos3Norm3Col4UV2(* const vertices)[], const size_t* const indexCounts, const uint32_t(* const indices)[], struct sMeshResources** const outMeshResources)
{
	// Before the CPU can reset the graphics command allocator for the current frame, it needs to wait until previous work stored in the allocator has completed on the GPU
	waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[currentFrameIndex], maxFenceWaitDurationMs);
	ID3D12CommandAllocator* graphicsCommandAllocator = graphicsCommandAllocators[currentFrameIndex].Get();
	fatalIfFailed(graphicsCommandAllocator->Reset());
	fatalIfFailed(graphicsCommandList->Reset(graphicsCommandAllocator, nullptr));

	// Create and write to upload heaps
	std::vector<ComPtr<ID3D12Resource>> uploadBuffers(meshCount * 2);
	for (uint32_t i = 0, j = 0; i < meshCount; ++i, j += 2)
	{
		const size_t meshVertexCount = vertexCounts[i];
		const sVertexPos3Norm3Col4UV2(*meshVertices)[] = vertices;
		const size_t meshIndexCount = indexCounts[i];
		const uint32_t(*meshIndices)[] = indices;
		sMeshResources& outMeshResource = *outMeshResources[i];

		const size_t vertexBufferWidth = sizeof(sVertexPos3Norm3Col4UV2) * meshVertexCount;
		const size_t indexBufferWidth = sizeof(uint32_t) * meshIndexCount;

		outMeshResource.indexCount = static_cast<uint32_t>(meshIndexCount);

		ComPtr<ID3D12Resource>& vertexUploadBuffer = uploadBuffers[j];
		createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_UPLOAD, vertexBufferWidth, D3D12_RESOURCE_STATE_GENERIC_READ, vertexUploadBuffer);
		updateBufferResource(vertexUploadBuffer.Get(), vertices, vertexBufferWidth);

		ComPtr<ID3D12Resource>& indexUploadBuffer = uploadBuffers[j + 1];
		createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_UPLOAD, indexBufferWidth, D3D12_RESOURCE_STATE_GENERIC_READ, indexUploadBuffer);
		updateBufferResource(indexUploadBuffer.Get(), indices, indexBufferWidth);

		// Create default heaps and submit copy upload heaps to default heaps work to the graphics queue
		createDefaultBufferAndRecordCopyCommand(graphicsCommandList.Get(), uploadBuffers[j].Get(), static_cast<UINT>(vertexBufferWidth), outMeshResource.vertexBufferResourceHandle);
		createDefaultBufferAndRecordCopyCommand(graphicsCommandList.Get(), uploadBuffers[j + 1].Get(), static_cast<UINT>(indexBufferWidth), outMeshResource.indexBufferResourceHandle);

		D3D12_RESOURCE_BARRIER resourceBarriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(resourceStore[outMeshResource.vertexBufferResourceHandle].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
			CD3DX12_RESOURCE_BARRIER::Transition(resourceStore[outMeshResource.indexBufferResourceHandle].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)
		};
		graphicsCommandList->ResourceBarrier(_countof(resourceBarriers), resourceBarriers);

		// Create buffer views
		createVertexBufferView(outMeshResource.vertexBufferResourceHandle, sizeof(sVertexPos3Norm3Col4UV2), static_cast<UINT>(vertexBufferWidth), outMeshResource.vertexBufferViewHandle);
		createIndexBufferView(outMeshResource.indexBufferResourceHandle, DXGI_FORMAT_R32_UINT, static_cast<UINT>(indexBufferWidth), outMeshResource.indexBufferViewHandle);
	}

	fatalIfFailed(graphicsCommandList->Close());
	ID3D12CommandList* commandLists[] = { graphicsCommandList.Get() };
	graphicsQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	graphicsFenceValues[currentFrameIndex] = ++graphicsFenceValue;
	graphicsQueue->Signal(graphicsFence.Get(), graphicsFenceValue);

	// Wait for the copy work to finish as the upload buffers are released after exiting this function
	waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[currentFrameIndex], maxFenceWaitDurationMs);
}

void direct3d12Graphics::loadShader(const std::string& shaderSourceFile, LPCWSTR entryPoint, LPCWSTR targetProfile, std::vector<uint8_t>& outBuffer)
{
	static const std::string compiledShaderFileExtension = "bin";

	const std::string shaderCompiledFile = fileIO::replaceExtension(shaderSourceFile, compiledShaderFileExtension);
	if (fileIO::fileExists(shaderCompiledFile))
	{
		fileIO::readSerializedBuffer(shaderCompiledFile, outBuffer);
	}
	else
	{
		ComPtr<IDxcBlob> blob;
		compileShader(std::wstring(shaderSourceFile.begin(), shaderSourceFile.end()).c_str(), entryPoint, targetProfile, blob);
		const size_t bufferSize = blob->GetBufferSize();
		outBuffer.resize(bufferSize);
		memcpy(outBuffer.data(), blob->GetBufferPointer(), bufferSize);
		fileIO::writeSerializedBuffer(shaderCompiledFile, outBuffer);
	}
}

void direct3d12Graphics::compileShader(LPCWSTR file, LPCWSTR entryPoint, LPCWSTR targetProfile, ComPtr<IDxcBlob>& outDxcBlob)
{
	std::string errorMessage;

	if (compileShaderFromFile(dxcLibrary.Get(), dxcCompiler.Get(), file, entryPoint, targetProfile, outDxcBlob, errorMessage) == 0)
	{
		platformLayer::messageBox::showMessageBoxFatal("direct3d12Graphics::compileShader: Failed to compile shader. " + errorMessage);
	}
}

void direct3d12Graphics::waitForGPU()
{
	// Wait for all frames to finish executing on the GPU
	const size_t graphicsFenceValueCount = graphicsFenceValues.size();
	for (size_t i = 0; i < graphicsFenceValueCount; ++i)
	{
		waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[i], maxFenceWaitDurationMs);
	}
}

void direct3d12Graphics::recordSurface(const direct3d12Surface* surface, ID3D12GraphicsCommandList6* commandList, const uint32_t renderDataCount, const struct sRenderData* const* renderData, const matrix4x4* const viewProjection)
{
	ID3D12Resource* const backBuffer = surface->renderTargetViews[surface->currentBackBufferIndex].Get();

	D3D12_RESOURCE_BARRIER backBufferResourceStartTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &backBufferResourceStartTransitionBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_rtvDescriptorHandle = surface->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cpu_rtvDescriptorHandle.ptr += (descriptorSizes.rtvDescriptorSize * surface->currentBackBufferIndex);
	const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.2f, 1.0f };
	commandList->ClearRenderTargetView(cpu_rtvDescriptorHandle, clearColor, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_dsvDescriptorHandle = surface->dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->ClearDepthStencilView(cpu_dsvDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->RSSetViewports(1, &surface->viewport);
	commandList->RSSetScissorRects(1, &surface->scissorRect);

	commandList->OMSetRenderTargets(1, &cpu_rtvDescriptorHandle, FALSE, &cpu_dsvDescriptorHandle);

	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(graphicsPipelineState.Get());

	// Update camera constants
	sCameraConstantBuffer cameraConstants = {};
	cameraConstantBuffer.update(&cameraConstants, sizeof(cameraConstants));
	commandList->SetGraphicsRootConstantBufferView(1, cameraConstantBuffer.GetGPUVirtualAddress());
	cameraConstantBuffer.increment();

	for (uint32_t i = 0; i < renderDataCount; ++i)
	{
		// Update object constants
		matrix4x4 worldViewProjectionMatrix = *renderData[i]->pWorldMatrix * *viewProjection;
		sObjectConstantBuffer objectConstants = {};
		// Copy matrix values cast to 32-bit floating point values
		std::transform(std::begin(worldViewProjectionMatrix.values), std::end(worldViewProjectionMatrix.values), std::begin(objectConstants.worldViewProjectionMatrix), [](const double& value) {return static_cast<float>(value); });
		objectConstantBuffer.update(&objectConstants, sizeof(objectConstants));
		commandList->SetGraphicsRootConstantBufferView(0, objectConstantBuffer.GetGPUVirtualAddress());
		objectConstantBuffer.increment();

		// Draw
		const sMeshResources* const mesh = renderData[i]->pMeshResources;
		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewStore[mesh->vertexBufferViewHandle]);
		commandList->IASetIndexBuffer(&indexBufferViewStore[mesh->indexBufferViewHandle]);
		commandList->DrawIndexedInstanced(mesh->indexCount, 1, 0, 0, 0);
	}

	D3D12_RESOURCE_BARRIER backBufferResourceEndTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &backBufferResourceEndTransitionBarrier);
}

void direct3d12Graphics::presentSurface(direct3d12Surface* surface, const bool useVSync, const bool tearingSupported)
{
	fatalIfFailed(surface->swapChain->Present(useVSync ? 1 : 0, ((tearingSupported) && (!useVSync)) ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

void direct3d12Graphics::createDefaultBufferAndRecordCopyCommand(ID3D12GraphicsCommandList6* commandList, ID3D12Resource* copySrcBffer, UINT64 width,
	size_t& outDefaultBufferResourceHandle)
{
	ComPtr<ID3D12Resource>& defaultBuffer = resourceStore.emplace_back(nullptr);
	outDefaultBufferResourceHandle = resourceStore.size() - 1;
	createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_DEFAULT, width, D3D12_RESOURCE_STATE_COPY_DEST, defaultBuffer);
	commandList->CopyBufferRegion(defaultBuffer.Get(), 0, copySrcBffer, 0, width);
}

void direct3d12Graphics::createVertexBufferView(const size_t vertexBufferResourceHandle, const UINT vertexStride, const UINT bufferWidth, size_t& outVertexBufferViewHandle)
{
	D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = vertexBufferViewStore.emplace_back();
	outVertexBufferViewHandle = vertexBufferViewStore.size() - 1;
	vertexBufferView.BufferLocation = resourceStore[vertexBufferResourceHandle]->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = vertexStride;
	vertexBufferView.SizeInBytes = bufferWidth;
}

void direct3d12Graphics::createIndexBufferView(const size_t indexBufferResourceHandle, const DXGI_FORMAT format, const UINT bufferWidth, size_t& outIndexBufferViewHandle)
{
	D3D12_INDEX_BUFFER_VIEW& indexBufferView = indexBufferViewStore.emplace_back();
	outIndexBufferViewHandle = indexBufferViewStore.size() - 1;
	indexBufferView.BufferLocation = resourceStore[indexBufferResourceHandle]->GetGPUVirtualAddress();
	indexBufferView.Format = format;
	indexBufferView.SizeInBytes = bufferWidth;
}
