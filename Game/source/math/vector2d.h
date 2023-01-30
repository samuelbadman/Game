#pragma once

class vector2d
{
public:
	float x;
	float y;

public:
	vector2d() : x(0.0f), y(0.0f) {}

	template <typename T>
	vector2d(const T inX, const T inY) : x(inX), y(inY) {}

public:
	std::string toString() const;
};