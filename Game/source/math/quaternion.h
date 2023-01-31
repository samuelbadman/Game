#pragma once

#include "vector4d.h"

class quaternion
{
public:
	double w;
	double x;
	double y;
	double z;

public:
	quaternion() : w(1.0), x(0.0), y(0.0), z(0.0) {}

	template <typename T>
	quaternion(const T inW, const T inX, const T inY, const T inZ)
		: w(static_cast<double>(inW)), x(static_cast<double>(inX)), y(static_cast<double>(inY)), z(static_cast<double>(inZ)) {}

public:
	vector4d operator*(const vector4d& rhs) const;
};