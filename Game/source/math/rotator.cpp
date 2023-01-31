#include "pch.h"
#include "rotator.h"

quaternion rotator::toQuaternion() const
{
    const glm::qua<double, glm::packed_highp> glmQuaternion = glm::qua<double, glm::packed_highp>(glm::radians(glm::vec<3, double, glm::packed_highp>(pitch, yaw, roll)));
    return quaternion(glmQuaternion.w, glmQuaternion.x, glmQuaternion.y, glmQuaternion.z);
}
