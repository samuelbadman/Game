#pragma once

// Standard library
#include <string>
#include <cstdarg>
#include <memory>
#include <chrono>
#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <filesystem>
#include <fstream>
#include <random>

#if defined(PLATFORM_WIN32)

// Windows libraries
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <shellapi.h>

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d12.h>
#include <dxcapi.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxcompiler.lib")
#include "platform/graphics/direct3D12/d3dx12.h"

#include <xaudio2.h>

#endif // PLATFORM_WIN32

// glm maths library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "math/glm_0.9.9.8/glm.hpp"
#include "math/glm_0.9.9.8/gtx/quaternion.hpp"
