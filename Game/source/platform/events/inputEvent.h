#pragma once

struct sInputEvent
{
	bool repeatedKey = false;
	int16_t input = 0;
	int32_t port = 0;
	float data = 0.f;
};