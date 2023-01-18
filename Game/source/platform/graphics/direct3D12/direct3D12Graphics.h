#pragma once

class direct3d12Graphics
{
public:
	static void init(bool useWarp, HWND hwnd, uint32_t width, uint32_t height, uint32_t backBufferCount);
};