#pragma once

#include "vector4d.h"

// 4x4 matrix stored in column-major order
class matrix4x4
{
public:
	vector4d columns[4] = { vector4d(), vector4d(), vector4d(), vector4d() };

public:
	matrix4x4() = default;
	matrix4x4(const vector4d& column0, const vector4d& column1, const vector4d& column2, const vector4d& column3);

	void operator=(const matrix4x4& rhs);

public:
	vector4d& operator[](const size_t index) { return columns[index]; }
	const vector4d& operator[](const size_t index) const { return columns[index]; }
	matrix4x4 operator*(const matrix4x4& rhs) const;

public:
	static matrix4x4 identity();
	static matrix4x4 inverse(const matrix4x4& a);
	static matrix4x4 transpose(const matrix4x4& a);

	// Computes a translation matrix from vector a
	static matrix4x4 translation(const class vector3d& a);

	// Computes a rotation matrix from rotator a
	static matrix4x4 rotation(const class rotator& a);

	// Computes a scale matrix from vector a
	static matrix4x4 scale(const class vector3d& a);

	// Computes a transformation (world/model) matrix from transform a
	static matrix4x4 transformation(const class transform& a);

	// Computes a view matrix from input position and rotation
	static matrix4x4 view(const class vector3d& position, const class rotator& inRotation);

	// Computes a perspective matrix from input parameters
	static matrix4x4 perspective(float fieldOfViewDegrees, float viewWidth, float viewHeight, float nearClipPlane, float farClipPlane);

	// Computes an orthographic matrix from input parameters
	static matrix4x4 orthographic(float width, float height, float nearClipPlane, float farClipPlane);

public:
	void inverseInPlace();
	void transposeInPlace();
};