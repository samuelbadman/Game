#include "pch.h"
#include "vector4d.h"
#include "string.h"

vector4d vector4d::normalize(const vector4d& a)
{
    glm::vec<4, double, glm::packed_highp> result = glm::normalize<4, double, glm::packed_highp>(glm::vec4(a.x, a.y, a.z, a.w));
    return vector4d(result.x, result.y, result.z, result.w);
}

std::string vector4d::toString() const
{
    return sString::printf("[x: %f, y: %f, z: %f, w: %f]", x, y, z, w);
}
