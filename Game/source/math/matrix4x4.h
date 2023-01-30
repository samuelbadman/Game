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

public:
	void inverseInPlace();
	void transposeInPlace();
};