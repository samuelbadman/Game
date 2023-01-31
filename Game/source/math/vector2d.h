#pragma once

class vector2d
{
public:
	double x = 0.0;
	double y = 0.0;

public:
	vector2d() = default;

	template <typename T>
	vector2d(const T inX, const T inY) : x(static_cast<double>(inX)), y(static_cast<double>(inY)) {}

public:
	std::string toString() const;
};