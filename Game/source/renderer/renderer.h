#pragma once

#include <memory>
#include <cstdint>

enum class rendererPlatform : uint8_t
{
	direct3d12 = 0,
	vulkan = 1
};

struct rendererInitSettings
{
	uint32_t displayIndex = 0;
};

class renderer
{
public:
	static std::unique_ptr<renderer> create(const rendererPlatform platform);

public:
	virtual rendererPlatform getPlatform() const = 0;
	virtual bool init(const rendererInitSettings& settings) = 0;
	virtual bool shutdown() = 0;
};