#pragma once

#include <memory>

enum class rendererPlatform : uint8_t
{
	direct3d12 = 0,
	vulkan = 1
};

class renderer
{
public:
	static std::unique_ptr<renderer> create(const rendererPlatform platform);

public:
	virtual rendererPlatform getPlatform() const = 0;
	virtual bool init() = 0;
	virtual bool shutdown() = 0;
};