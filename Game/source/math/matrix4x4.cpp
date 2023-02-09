#include "pch.h"
#include "matrix4x4.h"
#include "vector3d.h"
#include "rotator.h"
#include "transform.h"

static glm::mat4 asGlmMat(const matrix4x4& a)
{
	return glm::mat<4, 4, double, glm::packed_highp>(glm::vec<4, double, glm::packed_highp>(a.values[0], a.values[1], a.values[2], a.values[3]),
		glm::vec<4, double, glm::packed_highp>(a.values[4], a.values[5], a.values[6], a.values[7]),
		glm::vec<4, double, glm::packed_highp>(a.values[8], a.values[9], a.values[10], a.values[11]),
		glm::vec<4, double, glm::packed_highp>(a.values[12], a.values[13], a.values[14], a.values[15]));
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
	values[0] = column0.x;
	values[1] = column0.y;
	values[2] = column0.z;
	values[3] = column0.w;

	values[4] = column1.x;
	values[5] = column1.y;
	values[6] = column1.z;
	values[7] = column1.w;

	values[8] =  column2.x;
	values[9] =  column2.y;
	values[10] = column2.z;
	values[11] = column2.w;

	values[12] = column3.x;
	values[13] = column3.y;
	values[14] = column3.z;
	values[15] = column3.w;
}

void matrix4x4::operator=(const matrix4x4& rhs)
{
	memcpy(values, rhs.values, sizeof(double) * _countof(values));
}

matrix4x4 matrix4x4::operator*(const matrix4x4& rhs) const
{
	glm::mat<4, 4, double, glm::packed_highp> glmMat = asGlmMat(*this);
	glm::mat<4, 4, double, glm::packed_highp> rhsGlmMat = asGlmMat(rhs);
	return asMatrix4x4(glmMat * rhsGlmMat);
}

matrix4x4 matrix4x4::identity()
{
	static const glm::mat<4, 4, double, glm::packed_highp> identity = glm::identity<glm::mat<4, 4, double, glm::packed_highp>>();
	return asMatrix4x4(identity);
}

matrix4x4 matrix4x4::inverse(const matrix4x4& a)
{
	return asMatrix4x4(glm::inverse<4, 4, double, glm::packed_highp>(asGlmMat(a)));
}

matrix4x4 matrix4x4::transpose(const matrix4x4& a)
{
	return asMatrix4x4(glm::transpose<4, 4, double, glm::packed_highp>(asGlmMat(a)));
}

matrix4x4 matrix4x4::translation(const vector3d& a)
{
	static const glm::mat<4, 4, double, glm::packed_highp> identity = glm::identity<glm::mat<4, 4, double, glm::packed_highp>>();
	return asMatrix4x4(glm::translate<double, glm::packed_highp>(identity, glm::vec<3, double, glm::packed_highp>(a.x, a.y, a.z)));
}

matrix4x4 matrix4x4::rotation(const rotator& a)
{
	const quaternion aQuaternion = a.toQuaternion();
	return asMatrix4x4(glm::mat4_cast<double, glm::packed_highp>(glm::qua<double, glm::packed_highp>(aQuaternion.w, aQuaternion.x, aQuaternion.y, aQuaternion.z)));
}

matrix4x4 matrix4x4::scale(const vector3d& a)
{
	static const glm::mat<4, 4, double, glm::packed_highp> identity = glm::identity<glm::mat<4, 4, double, glm::packed_highp>>();
	return asMatrix4x4(glm::scale<double, glm::packed_highp>(identity, glm::vec<3, double, glm::packed_highp>(a.x, a.y, a.z)));
}

matrix4x4 matrix4x4::transformation(const transform& a)
{
	return matrix4x4::translation(a.position) * matrix4x4::rotation(a.rotation) * matrix4x4::scale(a.scale);
}

matrix4x4 matrix4x4::view(const vector3d& position, const rotator& inRotation)
{
	return inverse(translation(position) * rotation(inRotation));
}

matrix4x4 matrix4x4::perspective(double fieldOfViewDegrees, double viewWidth, double viewHeight, double nearClipPlane, double farClipPlane)
{
	return asMatrix4x4(glm::perspectiveFovLH<double>(glm::radians<double>(fieldOfViewDegrees), viewWidth, viewHeight, nearClipPlane, farClipPlane));
}

matrix4x4 matrix4x4::orthographic(double width, double height, double nearClipPlane, double farClipPlane)
{
	return asMatrix4x4(glm::orthoLH<double>(-width, width, -height, height, nearClipPlane, farClipPlane));
}

void matrix4x4::inverseInPlace()
{
	*this = inverse(*this);
}

void matrix4x4::transposeInPlace()
{
	*this = transpose(*this);
}
