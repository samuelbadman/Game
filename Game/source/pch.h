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

// Windows libraries
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d12.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")

#include <xaudio2.h>
