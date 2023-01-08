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
	std::wstring displayConnectedAdapterName = L"";
	bufferingType buffering = bufferingType::tripleBuffering;
	size_t graphicsContextSubmissionsPerFrameCount = 0;
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
	virtual void shutdown() = 0;
	virtual bool flush() = 0;
	virtual void submitRenderContexts(const renderCommand::commandContext commandContext, 
		const uint32_t numContexts, renderContext*const* contexts) = 0;
	virtual bool createRenderContext(const renderCommand::commandContext commandContext,
		std::unique_ptr<renderContext>& outRenderContext) const = 0;
	virtual bool destroyRenderContext(std::unique_ptr<renderContext>& outRenderContext) = 0;
};