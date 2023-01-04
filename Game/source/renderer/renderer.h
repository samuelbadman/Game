#pragma once

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

struct rendererInitSettings
{
	uint32_t displayIndex = 0;
	bufferingType buffering = bufferingType::tripleBuffering;
};

struct renderCommand
{
	enum class commandContext : uint8_t
	{
		graphics = 0,
		compute = 1,
		unknown
	};

	enum class commandType : uint8_t
	{
		beginFrame = 0,
		endFrame = 1
	};

	virtual ~renderCommand() = default;
	renderCommand(const commandContext inContext, const commandType inType) : context(inContext), type(inType)
	{}

	commandContext context;
	commandType type;
};

struct renderCommand_beginFrame : public renderCommand
{
	virtual ~renderCommand_beginFrame() final = default;
	renderCommand_beginFrame() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::beginFrame)
	{}
};

struct renderCommand_endFrame : public renderCommand
{
	virtual ~renderCommand_endFrame() final = default;
	renderCommand_endFrame() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::endFrame)
	{}
};

class renderer
{
public:
	static std::unique_ptr<renderer> create(const rendererPlatform platform);

public:
	virtual rendererPlatform getPlatform() const = 0;
	virtual bool init(const rendererInitSettings& settings) = 0;
	virtual bool shutdown() = 0;
	virtual void submitRenderCommand(const renderCommand& command) = 0;
};