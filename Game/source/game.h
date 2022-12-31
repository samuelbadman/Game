#pragma once

class game
{
public:
	void beginPlay();
	void tick(float deltaTime);
	void fixedTick(float step);
	void render();
};