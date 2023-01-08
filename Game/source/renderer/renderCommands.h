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

private:
	commandContext context;
	commandType type;
};

struct renderCommandBeginFrame : public renderCommand
{
	virtual ~renderCommandBeginFrame() final = default;
	renderCommandBeginFrame() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::beginFrame)
	{}
};

struct renderCommandEndFrame : public renderCommand
{
	virtual ~renderCommandEndFrame() final = default;
	renderCommandEndFrame() : renderCommand(renderCommand::commandContext::graphics, renderCommand::commandType::endFrame)
	{}
};