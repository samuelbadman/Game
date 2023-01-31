#pragma once

#include "math/vector3d.h"
#include "math/vector4d.h"
#include "math/vector2d.h"

struct sVertexPos3Norm3Col4UV2
{
	vector3d position = vector3d(0.0, 0.0, 0.0);
	vector3d normal = vector3d(0.0, 0.0, 0.0);
	vector4d color = vector4d(0.0, 0.0, 0.0, 0.0);
	vector2d uv = vector2d(0.0, 0.0);
};