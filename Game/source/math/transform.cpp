#include "pch.h"
#include "transform.h"
#include "matrix4x4.h"

static glm::mat4 asGlmMat(const matrix4x4& a)
{
	return glm::mat<4, 4, double, glm::packed_highp>(glm::vec<4, double, glm::packed_highp>(a.values[0], a.values[1], a.values[2], a.values[3]),
		glm::vec<4, double, glm::packed_highp>(a.values[4], a.values[5], a.values[6], a.values[7]),
		glm::vec<4, double, glm::packed_highp>(a.values[8], a.values[9], a.values[10], a.values[11]),
		glm::vec<4, double, glm::packed_highp>(a.values[12], a.values[13], a.values[14], a.values[15]));
}

transform transform::toTransform(const matrix4x4& a)
{
	// Decompose world matrix into translation, rotation and scale
	const glm::mat<4, 4, double, glm::packed_highp> glmMat = asGlmMat(a);
	const glm::vec<3, double, glm::packed_highp> glmTranslation = glmMat[3];

	glm::vec<3, double, glm::packed_highp> glmScale;
	for (int32_t i = 0; i < 3; ++i)
	{
		glmScale[i] = glm::length<3, double, glm::packed_highp>(glm::vec<3, double, glm::packed_highp>(glmMat[i]));
	}

	const glm::vec<3, double, glm::packed_highp> glmRotationEuler = glm::degrees<3, double, glm::packed_highp>(glm::eulerAngles(glm::quat_cast(glm::mat<3, 3, double, glm::packed_highp>(
		glm::vec<3, double, glm::packed_highp>(glmMat[0]) / glmScale[0],
		glm::vec<3, double, glm::packed_highp>(glmMat[1]) / glmScale[1],
		glm::vec<3, double, glm::packed_highp>(glmMat[2]) / glmScale[2]))));

	return transform(vector3d(glmTranslation.x, glmTranslation.y, glmTranslation.z), 
		rotator(glmRotationEuler.x, glmRotationEuler.y, glmRotationEuler.z), 
		vector3d(glmScale.x, glmScale.y, glmScale.z));
}
