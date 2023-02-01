#include "pch.h"

#include "direct3d12Graphics.h"
#include "platform/framework/platformMessageBox.h"
#include "direct3d12Surface.h"
#include "fileIO/fileIO.h"
#include "platform/graphics/vertexPos3Norm3Col4UV2.h"
#include "platform/graphics/meshResources.h"

using namespace Microsoft::WRL;

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
		platformMessageBoxFatal("direct3d12Graphics::CreateEvent: failed to create event handle.");
	}
}

static void waitForFence(ID3D12Fence* fence, HANDLE inEventHandle, uint64_t value, DWORD waitDuration)
{
	if (fence->GetCompletedValue() < value)
	{
		fatalIfFailed(fence->SetEventOnCompletion(value, inEventHandle));
		if (WaitForSingleObject(inEventHandle, waitDuration) == WAIT_FAILED)
		{
			platformMessageBoxFatal("direct3d12Graphics::waitForFence: failed to wait for single object.");
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

static constexpr DWORD maxFenceWaitDurationMs = static_cast<DWORD>(std::chrono::milliseconds::max().count());

uint32_t direct3d12Graphics::backBufferCount = 0;
ComPtr<IDXGIFactory7> direct3d12Graphics::dxgiFactory;
ComPtr<IDXGIAdapter4> direct3d12Graphics::adapter;
ComPtr<ID3D12Device8> direct3d12Graphics::device;
ComPtr<ID3D12CommandQueue> direct3d12Graphics::graphicsQueue;
ComPtr<ID3D12CommandQueue> direct3d12Graphics::computeQueue;
ComPtr<ID3D12CommandQueue> direct3d12Graphics::copyQueue;
sDescriptorSizes direct3d12Graphics::descriptorSizes = {};
std::vector<ComPtr<ID3D12CommandAllocator>> direct3d12Graphics::graphicsCommandAllocators;
ComPtr<ID3D12GraphicsCommandList6> direct3d12Graphics::graphicsCommandList;
ComPtr<ID3D12Fence> direct3d12Graphics::graphicsFence;
uint64_t direct3d12Graphics::graphicsFenceValue = 0;
std::vector<uint64_t> direct3d12Graphics::graphicsFenceValues;
HANDLE direct3d12Graphics::eventHandle = nullptr;
uint32_t direct3d12Graphics::currentFrameIndex = 0;

ComPtr<IDxcLibrary> direct3d12Graphics::dxcLibrary;
ComPtr<IDxcCompiler> direct3d12Graphics::dxcCompiler;

ComPtr<ID3D12RootSignature> direct3d12Graphics::rootSig;
ComPtr<ID3D12PipelineState> direct3d12Graphics::graphicsPipelineState;

std::vector<ComPtr<ID3D12Resource>> direct3d12Graphics::resourceStore;

void direct3d12Graphics::init(bool useWarp, uint32_t inBackBufferCount)
{
	backBufferCount = inBackBufferCount;

	enableDebugLayer();
	createDxgiFactory(dxgiFactory);
	getAdapter(dxgiFactory.Get(), false, adapter);
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
	graphicsFenceValues.resize(static_cast<size_t>(backBufferCount), graphicsFenceValue);

	createEventHandle(eventHandle);

	createDxcLibrary(dxcLibrary);
	createDxcCompiler(dxcCompiler);

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
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.NumParameters = 0;
	rootSigDesc.pParameters = nullptr;
	rootSigDesc.NumStaticSamplers = 0;
	rootSigDesc.pStaticSamplers = nullptr;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;

	ID3DBlob* signature;
	fatalIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, nullptr));
	fatalIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSig)));

	// Shader compilation
	std::vector<uint8_t> vertexShaderBuffer;
	loadShader("shaders/vertexShader.hlsl", L"main", L"vs_6_0", vertexShaderBuffer);

	std::vector<uint8_t> pixelShaderBuffer;
	loadShader("shaders/pixelShader.hlsl", L"main", L"ps_6_0", pixelShaderBuffer);

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
	graphicsPipelineStateDesc.pRootSignature = rootSig.Get();
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
}

void direct3d12Graphics::createSurface(void* hwnd, uint32_t width, uint32_t height, std::shared_ptr<graphicsSurface>& outSurface)
{
	outSurface = std::make_shared<graphicsSurface>();
	createSwapChain(dxgiFactory.Get(), graphicsQueue.Get(), static_cast<HWND>(hwnd), width, height, backBufferCount, outSurface->swapChain);
	createDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, backBufferCount, false, outSurface->rtvDescriptorHeap);
	outSurface->renderTargetViews.resize(static_cast<size_t>(backBufferCount));
	updateRenderTargetViews(device.Get(),
		outSurface->swapChain.Get(), 
		descriptorSizes.rtvDescriptorSize, 
		outSurface->renderTargetViews, 
		outSurface->rtvDescriptorHeap.Get(), 
		backBufferCount);
	createDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false, outSurface->dsvDescriptorHeap);
	updateDepthStencilView(device.Get(), width, height, outSurface->depthStencilView, outSurface->dsvDescriptorHeap.Get());

	// Surfaces currently only support a single viewport. There is currently no support for splitscreen
	D3D12_VIEWPORT& outViewport = outSurface->viewport;
	outViewport.Width = static_cast<FLOAT>(width);
	outViewport.Height = static_cast<FLOAT>(height);
	outViewport.TopLeftX = 0.0f;
	outViewport.TopLeftY = 0.0f;
	outViewport.MinDepth = 0.0f;
	outViewport.MaxDepth = 1.0f;
	D3D12_RECT& outScissorRect = outSurface->scissorRect;
	outScissorRect.top = 0;
	outScissorRect.left = 0;
	outScissorRect.right = width;
	outScissorRect.bottom = height;
}

void direct3d12Graphics::destroySurface(std::shared_ptr<graphicsSurface>& surface)
{
	waitForGPU();

	surface->swapChain.Reset();
	surface->renderTargetViews.clear();
	surface->rtvDescriptorHeap.Reset();
	surface->depthStencilView.Reset();
	surface->dsvDescriptorHeap.Reset();

	surface.reset();
}

void direct3d12Graphics::resizeSurface(graphicsSurface* surface, uint32_t width, uint32_t height)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	fatalIfFailed(surface->swapChain->GetDesc(&swapChainDesc));

	// Don't resize if the width and height have not changed
	if (swapChainDesc.BufferDesc.Width != width || swapChainDesc.BufferDesc.Height != height)
	{
		// Don't allow 0 size swap chain back buffers
		width = std::max(1u, width);
		height = std::max(1u, height);

		// Wait for all frames to finish executing on the GPU
		waitForGPU();

		// Release render target view resources
		const size_t backBufferCount = surface->renderTargetViews.size();
		for (size_t i = 0; i < backBufferCount; ++i)
		{
			surface->renderTargetViews[i].Reset();
			graphicsFenceValues[i] = graphicsFenceValues[currentFrameIndex];
		}

		// Resize the swap chain back buffers
		fatalIfFailed(surface->swapChain->ResizeBuffers(static_cast<UINT>(backBufferCount), width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		// Update current back buffer index
		currentFrameIndex = surface->swapChain->GetCurrentBackBufferIndex();

		// Update render target view resources with new back buffers
		updateRenderTargetViews(device.Get(),
			surface->swapChain.Get(), 
			descriptorSizes.rtvDescriptorSize, 
			surface->renderTargetViews,
			surface->rtvDescriptorHeap.Get(),
			static_cast<UINT>(backBufferCount));

		// Update depth stencil view resource
		updateDepthStencilView(device.Get(), width, height, surface->depthStencilView, surface->dsvDescriptorHeap.Get());

		// Update viewport
		D3D12_VIEWPORT& outViewport = surface->viewport;
		outViewport.Width = static_cast<FLOAT>(width);
		outViewport.Height = static_cast<FLOAT>(height);

		// Update scissor rect
		D3D12_RECT& outScissorRect = surface->scissorRect;
		outScissorRect.right = width;
		outScissorRect.bottom = height;
	}
}

void direct3d12Graphics::render(const uint32_t numSurfaces, graphicsSurface* const * surfaces, const bool useVSync)
{
	// Wait for the previous frame to finish on the GPU
	waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[currentFrameIndex], maxFenceWaitDurationMs);

	// Get frame resources
	ID3D12CommandAllocator* const graphicsCommandAllocator = graphicsCommandAllocators[currentFrameIndex].Get();

	// Start recording command list
	fatalIfFailed(graphicsCommandAllocator->Reset());
	fatalIfFailed(graphicsCommandList->Reset(graphicsCommandAllocator, nullptr));

	// For each surface
	for(uint32_t i = 0; i < numSurfaces; ++i)
	{
		const graphicsSurface* const surface = surfaces[static_cast<size_t>(i)];
		recordSurface(surface, graphicsCommandList.Get());
	}

	// Stop recording command list
	fatalIfFailed(graphicsCommandList->Close());

	// Execute command lists
	ID3D12CommandList* graphicsExecuteLists[1] = { graphicsCommandList.Get() };
	graphicsQueue->ExecuteCommandLists(_countof(graphicsExecuteLists), graphicsExecuteLists);

	// Present each surface
	static bool tearingSupported = checkTearingSupport(dxgiFactory.Get());
	for (uint32_t i = 0; i < numSurfaces; ++i)
	{
		const graphicsSurface* surface = surfaces[static_cast<size_t>(i)];
		presentSurface(surface, useVSync, tearingSupported);
	}

	// Signal end frame. Must be done after present as flip discard swap effect is being used and this presents without blocking CPU thread
	++graphicsFenceValue;
	graphicsFenceValues[currentFrameIndex] = graphicsFenceValue;
	fatalIfFailed(graphicsQueue->Signal(graphicsFence.Get(), graphicsFenceValues[currentFrameIndex]));
}

void direct3d12Graphics::loadMesh(const size_t vertexCount, 
	const sVertexPos3Norm3Col4UV2* const vertices, 
	const size_t indexCount, 
	const uint32_t* const indices, 
	sMeshResources& outMeshResources)
{
	const size_t vertexBufferWidth = sizeof(sVertexPos3Norm3Col4UV2) * vertexCount;
	const size_t indexBufferWidth = sizeof(uint32_t) * indexCount;

	static const auto updateBufferResource = [](ID3D12Resource* buffer, const void* src, const size_t width) {
		D3D12_RANGE readRange = {};
		void* resourceData;
		fatalIfFailed(buffer->Map(0, &readRange, &resourceData));
		memcpy(resourceData, src, width);
		buffer->Unmap(0, nullptr);
	};

	// Create and write to upload heaps
	ComPtr<ID3D12Resource> vertexUploadBuffer;
	createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_UPLOAD, vertexBufferWidth, D3D12_RESOURCE_STATE_GENERIC_READ, vertexUploadBuffer);
	updateBufferResource(vertexUploadBuffer.Get(), vertices, vertexBufferWidth);

	ComPtr<ID3D12Resource> indexUploadBuffer;
	createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_UPLOAD, indexBufferWidth, D3D12_RESOURCE_STATE_GENERIC_READ, indexUploadBuffer);
	updateBufferResource(indexUploadBuffer.Get(), indices, indexBufferWidth);


	// Create default heaps and submit copy upload heaps to default heaps work to the graphics queue
	// Before the CPU can reset the graphics command allocator for the current frame, it needs to wait until previous work stored in the allocator has completed on the GPU
	waitForFence(graphicsFence.Get(), eventHandle, graphicsFenceValues[currentFrameIndex], maxFenceWaitDurationMs);
	ID3D12CommandAllocator* graphicsCommandAllocator = graphicsCommandAllocators[currentFrameIndex].Get();
	fatalIfFailed(graphicsCommandAllocator->Reset());
	fatalIfFailed(graphicsCommandList->Reset(graphicsCommandAllocator, nullptr));

	{
		ComPtr<ID3D12Resource>& vertexDefaultBuffer = resourceStore.emplace_back(nullptr);
		createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_DEFAULT, vertexBufferWidth, D3D12_RESOURCE_STATE_COPY_DEST, vertexDefaultBuffer);
		outMeshResources.vertexBufferHandle = resourceStore.size() - 1;
		graphicsCommandList->CopyBufferRegion(vertexDefaultBuffer.Get(), 0, vertexUploadBuffer.Get(), 0, vertexBufferWidth);
	}

	{
		ComPtr<ID3D12Resource>& indexDefaultBuffer = resourceStore.emplace_back(nullptr);
		createCommittedBuffer(device.Get(), D3D12_HEAP_TYPE_DEFAULT, indexBufferWidth, D3D12_RESOURCE_STATE_COPY_DEST, indexDefaultBuffer);
		outMeshResources.indexBufferHandle = resourceStore.size() - 1;
		graphicsCommandList->CopyBufferRegion(indexDefaultBuffer.Get(), 0, indexUploadBuffer.Get(), 0, indexBufferWidth);
	}

	D3D12_RESOURCE_BARRIER resourceBarriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(resourceStore[outMeshResources.vertexBufferHandle].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
		CD3DX12_RESOURCE_BARRIER::Transition(resourceStore[outMeshResources.indexBufferHandle].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)
	};
	graphicsCommandList->ResourceBarrier(_countof(resourceBarriers), resourceBarriers);

	fatalIfFailed(graphicsCommandList->Close());
	ID3D12CommandList* commandLists[] = { graphicsCommandList.Get() };
	graphicsQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	graphicsFenceValues[currentFrameIndex] = ++graphicsFenceValue;
	graphicsQueue->Signal(graphicsFence.Get(), graphicsFenceValue);
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
		platformMessageBoxFatal("direct3d12Graphics::compileShader: Failed to compile shader. " + errorMessage);
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

void direct3d12Graphics::recordSurface(const graphicsSurface* surface, ID3D12GraphicsCommandList6* commandList)
{
	ID3D12Resource* const backBuffer = surface->renderTargetViews[currentFrameIndex].Get();

	D3D12_RESOURCE_BARRIER backBufferResourceStartTransitionBarrier = {};
	backBufferResourceStartTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBufferResourceStartTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBufferResourceStartTransitionBarrier.Transition.pResource = backBuffer;
	backBufferResourceStartTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBufferResourceStartTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	backBufferResourceStartTransitionBarrier.Transition.Subresource = 0;

	graphicsCommandList->ResourceBarrier(1, &backBufferResourceStartTransitionBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_rtvDescriptorHandle = surface->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cpu_rtvDescriptorHandle.ptr += (descriptorSizes.rtvDescriptorSize * currentFrameIndex);
	const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.4f, 1.0f };
	graphicsCommandList->ClearRenderTargetView(cpu_rtvDescriptorHandle, clearColor, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_dsvDescriptorHandle = surface->dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	graphicsCommandList->ClearDepthStencilView(cpu_dsvDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	graphicsCommandList->RSSetViewports(1, &surface->viewport);
	graphicsCommandList->RSSetScissorRects(1, &surface->scissorRect);

	// Todo: Receive as function argument an array of render data for each surface describing what to render onto each surface

	D3D12_RESOURCE_BARRIER backBufferResourceEndTransitionBarrier = {};
	backBufferResourceEndTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBufferResourceEndTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBufferResourceEndTransitionBarrier.Transition.pResource = backBuffer;
	backBufferResourceEndTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	backBufferResourceEndTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBufferResourceEndTransitionBarrier.Transition.Subresource = 0;

	graphicsCommandList->ResourceBarrier(1, &backBufferResourceEndTransitionBarrier);
}

void direct3d12Graphics::presentSurface(const graphicsSurface* surface, const bool useVSync, const bool tearingSupported)
{
	fatalIfFailed(surface->swapChain->Present(useVSync ? 1 : 0, ((tearingSupported) && (!useVSync)) ? DXGI_PRESENT_ALLOW_TEARING : 0));
	currentFrameIndex = surface->swapChain->GetCurrentBackBufferIndex();
}

