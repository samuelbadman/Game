#include "pch.h"
#include "vector3d.h"
#include "stringHelper.h"

vector3d vector3d::operator=(const vector3d& rhs)
{
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
}

vector3d vector3d::operator-(const vector3d& rhs) const
{
    glm::vec3 result = glm::vec3(x, y, z) - glm::vec3(rhs.x, rhs.y, rhs.z);
    return vector3d(result.x, result.y, result.z);
}

vector3d vector3d::operator+(const vector3d& rhs) const
{
    glm::vec3 result = glm::vec3(x, y, z) + glm::vec3(rhs.x, rhs.y, rhs.z);
    return vector3d(result.x, result.y, result.z);
}

vector3d vector3d::crossProduct(const vector3d& a, const vector3d& b)
{
    glm::vec3 result = glm::cross(glm::vec3(a.x, a.y, a.z), glm::vec3(b.x, b.y, b.z));
    return vector3d(result.x, result.y, result.z);
}

float vector3d::dotProduct(const vector3d& a, const vector3d& b)
{
    return glm::dot(glm::vec3(a.x, a.y, a.z), glm::vec3(b.x, b.y, b.z));
}

vector3d vector3d::normalize(const vector3d& a)
{
    glm::vec3 result = glm::normalize(glm::vec3(a.x, a.y, a.z));
    return vector3d(result.x, result.y, result.z);
}

void vector3d::normalizeInPlace()
{
    *this = normalize(*this);
}

std::string vector3d::toString() const
{
    return stringHelper::printf("[x: %f, y: %f, z: %f]", x, y, z);
}

vector3d operator*(const float lhs, const vector3d& rhs)
{
    glm::vec3 result = lhs * glm::vec3(rhs.x, rhs.y, rhs.z);
    return vector3d(result.x, result.y, result.z);
}
