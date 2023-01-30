#include "pch.h"
#include "vector4d.h"
#include "stringHelper.h"

vector4d vector4d::normalize(const vector4d& a)
{
    glm::vec4 result = glm::normalize(glm::vec4(a.x, a.y, a.z, a.w));
    return vector4d(result.x, result.y, result.z, result.w);
}

std::string vector4d::toString() const
{
    return stringHelper::printf("[x: %f, y: %f, z: %f, w: %f]", x, y, z, w);
}
