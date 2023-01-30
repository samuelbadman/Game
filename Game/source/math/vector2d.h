#pragma once

class vector2d
{
public:
	float x = 0.0f;
	float y = 0.0f;

public:
	vector2d() = default;

	template <typename T>
	vector2d(const T inX, const T inY) : x(static_cast<float>(inX)), y(static_cast<float>(inY)) {}

public:
	std::string toString() const;
};