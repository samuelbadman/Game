#include "pch.h"
#include "quaternion.h"

vector4d quaternion::operator*(const vector4d& rhs) const
{
    glm::vec<4, double, glm::packed_highp> result = glm::qua<double, glm::packed_highp>(w, x, y, z) * glm::vec<4, double, glm::packed_highp>(rhs.x, rhs.y, rhs.z, rhs.w);
    return vector4d(result.x, result.y, result.z, result.w);
}
