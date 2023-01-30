#include "pch.h"
#include "quaternion.h"

vector4d quaternion::operator*(const vector4d& rhs) const
{
    glm::vec4 result = glm::quat(w, x, y, z) * glm::vec4(rhs.x, y, z, w);
    return vector4d(result.x, result.y, result.z, result.w);
}
