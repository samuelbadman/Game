#include "pch.h"
#include "quaternion.h"

static matrix4x4 asMatrix4x4(const glm::mat4& a)
{
	return matrix4x4(vector4d(a[0].x, a[0].y, a[0].z, a[0].w),
		vector4d(a[1].x, a[1].y, a[1].z, a[1].w),
		vector4d(a[2].x, a[2].y, a[2].z, a[2].w),
		vector4d(a[3].x, a[3].y, a[3].z, a[3].w));
}

vector4d quaternion::operator*(const vector4d& rhs) const
{
    glm::vec4 result = glm::quat(w, x, y, z) * glm::vec4(rhs.x, y, z, w);
    return vector4d(result.x, result.y, result.z, result.w);
}

matrix4x4 quaternion::toMatrix4x4() const
{
	return asMatrix4x4(glm::mat4_cast(glm::quat(w, x, y, z)));
}
