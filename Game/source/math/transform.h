#pragma once

#include "vector3d.h"
#include "rotator.h"

class transform
{
public:
	vector3d position;
	rotator rotation;
	vector3d scale;

public:
	transform() = default;
	transform(const vector3d& inPosition, const rotator& inRotator, const vector3d& inScale)
		: position(inPosition), rotation(inRotator), scale(inScale) {}

public:
	// Converts the matrix into a transform. Assumes the input matrix contians a transformation, i.e is a world/model matrix
	static transform toTransform(const class matrix4x4& a);
};