#pragma once

#include "events/core/inputEvent.h"

class game
{
public:
	void beginPlay();
	void tick(float deltaTime);
	void fixedTick(float step);
	void render();
	void onMouseKeyboardInput(const inputEvent& event);
	void onGamepadInput(const inputEvent& event);
};