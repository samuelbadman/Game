#pragma once

#include <string>
#include <cstdarg>
#include <memory>
#include <chrono>
#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d12.h>
#include <wrl.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#include "renderer/d3d12/d3dx12.h"
