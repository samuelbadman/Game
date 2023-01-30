#include "pch.h"
#include "rotator.h"

quaternion rotator::toQuaternion() const
{
    glm::quat glmQuaternion = glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));
    return quaternion(glmQuaternion.w, glmQuaternion.x, glmQuaternion.y, glmQuaternion.z);
}
