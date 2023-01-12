#pragma once

// Render command base
struct renderCommand
{
	enum class commandContext : uint8_t
	{
		graphics = 0,
		unknown
	};

	enum class commandType : uint8_t
	{
		beginContext = 0,
		endContext = 1,
		beginFrame = 2,
		endFrame = 3
	};

	virtual ~renderCommand() = default;
	renderCommand(const commandContext inContext, const commandType inType) : context(inContext), type(inType)
	{}

	commandContext getContext() const { return context; }
	commandType getType() const { return type; }

private:
	commandContext context;
	commandType type;
};

// ---------------------------------------------
// Render commands
// ---------------------------------------------
struct renderCommand_beginContext : public renderCommand
{
	renderCommand_beginContext() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::beginContext)
	{}

	uint32_t frameIndex = 0;
};

struct renderCommand_endContext : public renderCommand
{
	renderCommand_endContext() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::endContext)
	{}
};

struct renderCommand_beginFrame : public renderCommand
{
	renderCommand_beginFrame() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::beginFrame)
	{}
};

struct renderCommand_endFrame : public renderCommand
{
	renderCommand_endFrame() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::endFrame)
	{}
};