#pragma once

#include "renderCommands.h"

enum class rendererPlatform : uint8_t
{
	direct3d12 = 0,
	vulkan = 1
};

enum class bufferingType : uint8_t
{
	doubleBuffering = 0,
	tripleBuffering = 1
};

struct renderDeviceInitSettings
{
	uint32_t displayIndex = 0;
	bufferingType buffering = bufferingType::tripleBuffering;
};

struct renderContextInitSettings
{
	renderCommand::commandContext context = renderCommand::commandContext::unknown;
};

class renderContext
{
private:
	renderCommand::commandContext context = renderCommand::commandContext::unknown;

public:
	virtual ~renderContext() = default;

public:
	virtual rendererPlatform getPlatform() const = 0;
	virtual void submitRenderCommand(const renderCommand& command) = 0;

public:
	renderCommand::commandContext getCommandContext() const { return context; }

protected:
	void setCommandContext(renderCommand::commandContext inContext) { context = inContext; }
};

class renderDevice
{
public:
	static std::unique_ptr<renderDevice> create(const rendererPlatform platform);

public:
	virtual ~renderDevice() = default;

public:
	virtual rendererPlatform getPlatform() const = 0;
	virtual bool init(const renderDeviceInitSettings& settings) = 0;
	virtual bool shutdown() = 0;
	virtual bool flush() = 0;
};