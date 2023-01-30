#include "pch.h"
#include "matrix4x4.h"

static glm::mat4 asGlmMat(const matrix4x4& a)
{
	return glm::mat4(glm::vec4(a.columns[0].x, a.columns[0].y, a.columns[0].z, a.columns[0].w),
		glm::vec4(a.columns[1].x, a.columns[1].y, a.columns[1].z, a.columns[1].w),
		glm::vec4(a.columns[2].x, a.columns[2].y, a.columns[2].z, a.columns[2].w),
		glm::vec4(a.columns[3].x, a.columns[3].y, a.columns[3].z, a.columns[3].w));
}

static matrix4x4 asMatrix4x4(const glm::mat4& a)
{
	return matrix4x4(vector4d(a[0].x, a[0].y, a[0].z, a[0].w),
		vector4d(a[1].x, a[1].y, a[1].z, a[1].w),
		vector4d(a[2].x, a[2].y, a[2].z, a[2].w),
		vector4d(a[3].x, a[3].y, a[3].z, a[3].w));
}

matrix4x4::matrix4x4(const vector4d& column0, const vector4d& column1, const vector4d& column2, const vector4d& column3)
{
	columns[0] = column0;
	columns[1] = column1;
	columns[2] = column2;
	columns[3] = column3;
}

void matrix4x4::operator=(const matrix4x4& rhs)
{
	columns[0] = rhs.columns[0];
	columns[1] = rhs.columns[1];
	columns[2] = rhs.columns[2];
	columns[3] = rhs.columns[3];
}

matrix4x4 matrix4x4::operator*(const matrix4x4& rhs) const
{
	glm::mat4 glmMat = asGlmMat(*this);
	glm::mat4 rhsGlmMat = asGlmMat(rhs);
	return asMatrix4x4(glmMat * rhsGlmMat);
}

matrix4x4 matrix4x4::identity()
{
	return asMatrix4x4(glm::identity<glm::mat4>());
}

matrix4x4 matrix4x4::inverse(const matrix4x4& a)
{
	return asMatrix4x4(glm::inverse(asGlmMat(a)));
}

matrix4x4 matrix4x4::transpose(const matrix4x4& a)
{
	return asMatrix4x4(glm::transpose(asGlmMat(a)));
}

void matrix4x4::inverseInPlace()
{
	*this = inverse(*this);
}

void matrix4x4::transposeInPlace()
{
	*this = transpose(*this);
}
