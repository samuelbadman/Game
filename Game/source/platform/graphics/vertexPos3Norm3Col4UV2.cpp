#include "pch.h"
#include "vertexPos3Norm3Col4UV2.h"
#include "math/vector3d.h"
#include "math/vector4d.h"
#include "math/vector2d.h"

sVertexPos3Norm3Col4UV2::sVertexPos3Norm3Col4UV2(const vector3d& inPosition, const vector3d& inNormal, const vector4d& inColor, const vector2d& inUV)
{
	position[0] = static_cast<float>(inPosition.x);
	position[1] = static_cast<float>(inPosition.y);
	position[2] = static_cast<float>(inPosition.z);

	normal[0] = static_cast<float>(inNormal.x);
	normal[1] = static_cast<float>(inNormal.y);
	normal[2] = static_cast<float>(inNormal.z);

	color[0] = static_cast<float>(inColor.x);
	color[1] = static_cast<float>(inColor.y);
	color[2] = static_cast<float>(inColor.z);
	color[3] = static_cast<float>(inColor.w);

	uv[0] = static_cast<float>(inUV.x);
	uv[1] = static_cast<float>(inUV.y);
}
