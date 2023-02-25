#include "pch.h"
#include "vector3d.h"
#include "string.h"

void vector3d::operator=(const vector3d& rhs)
{
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
}

vector3d vector3d::operator-(const vector3d& rhs) const
{
    glm::vec<3, double, glm::packed_highp> result = glm::vec<3, double, glm::packed_highp>(x, y, z) - glm::vec<3, double, glm::packed_highp>(rhs.x, rhs.y, rhs.z);
    return vector3d(result.x, result.y, result.z);
}

vector3d vector3d::operator+(const vector3d& rhs) const
{
    glm::vec<3, double, glm::packed_highp> result = glm::vec<3, double, glm::packed_highp>(x, y, z) + glm::vec<3, double, glm::packed_highp>(rhs.x, rhs.y, rhs.z);
    return vector3d(result.x, result.y, result.z);
}

vector3d vector3d::crossProduct(const vector3d& a, const vector3d& b)
{
    glm::vec<3, double, glm::packed_highp> result = glm::cross<double, glm::packed_highp>(glm::vec3(a.x, a.y, a.z), glm::vec3(b.x, b.y, b.z));
    return vector3d(result.x, result.y, result.z);
}

double vector3d::dotProduct(const vector3d& a, const vector3d& b)
{
    return glm::dot<3, double, glm::packed_highp>(glm::vec3(a.x, a.y, a.z), glm::vec3(b.x, b.y, b.z));
}

vector3d vector3d::normalize(const vector3d& a)
{
    glm::vec<3, double, glm::packed_highp> result = glm::normalize<3, double, glm::packed_highp>(glm::vec3(a.x, a.y, a.z));
    return vector3d(result.x, result.y, result.z);
}

void vector3d::normalizeInPlace()
{
    *this = normalize(*this);
}

std::string vector3d::toString() const
{
    return sString::printf("[x: %f, y: %f, z: %f]", x, y, z);
}

vector3d operator*(const double lhs, const vector3d& rhs)
{
    glm::vec<3, double, glm::packed_highp> result = lhs * glm::vec<3, double, glm::packed_highp>(rhs.x, rhs.y, rhs.z);
    return vector3d(result.x, result.y, result.z);
}
