#include "pch.h"
#include "transform.h"
#include "matrix4x4.h"

static glm::mat4 asGlmMat(const matrix4x4& a)
{
	return glm::mat<4, 4, double, glm::packed_highp>(glm::vec<4, double, glm::packed_highp>(a.columns[0].x, a.columns[0].y, a.columns[0].z, a.columns[0].w),
		glm::vec<4, double, glm::packed_highp>(a.columns[1].x, a.columns[1].y, a.columns[1].z, a.columns[1].w),
		glm::vec<4, double, glm::packed_highp>(a.columns[2].x, a.columns[2].y, a.columns[2].z, a.columns[2].w),
		glm::vec<4, double, glm::packed_highp>(a.columns[3].x, a.columns[3].y, a.columns[3].z, a.columns[3].w));
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
