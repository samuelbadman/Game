#include "pch.h"
#include "direct3d12Graphics.h"
#include "platform/framework/win32/win32MessageBox.h"

#define fatalIfFailed(x) try { if(FAILED(x)) throw std::runtime_error("hresult failed."); }\
catch(const std::runtime_error& exception){ win32MessageBox::messageBoxFatal(exception.what()); }

static void enableDebugLayer()
{
#if defined(_DEBUG)
	// Enable the d3d12 debug layer
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
	fatalIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif // _DEBUG
}

void direct3d12Graphics::init()
{
	enableDebugLayer();
}
