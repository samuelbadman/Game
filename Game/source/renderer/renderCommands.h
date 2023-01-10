#pragma once

struct renderCommand
{
	enum class commandContext : uint8_t
	{
		graphics = 0,
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

	commandContext getContext() const { return context; }
	commandType getType() const { return type; }

private:
	commandContext context;
	commandType type;
};

struct renderCommand_beginContext : public renderCommand
{
	virtual ~renderCommand_beginContext() final = default;
	renderCommand_beginContext() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::beginFrame)
	{}

	uint8_t frameIndex = 0;
};

struct renderCommand_endContext : public renderCommand
{
	virtual ~renderCommand_endContext() final = default;
	renderCommand_endContext() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::endFrame)
	{}
};