#pragma once

#include "vector4d.h"
#include "matrix4x4.h"

class quaternion
{
public:
	float w;
	float x;
	float y;
	float z;

public:
	quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}

	template <typename T>
	quaternion(const T inW, const T inX, const T inY, const T inZ)
		: w(static_cast<float>(inW)), x(static_cast<float>(inX)), y(static_cast<float>(inY)), z(static_cast<float>(inZ)) {}

public:
	vector4d operator*(const vector4d& rhs) const;

public:
	matrix4x4 toMatrix4x4() const;
};