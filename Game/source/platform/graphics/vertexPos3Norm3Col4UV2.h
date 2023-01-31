#pragma once

struct sVertexPos3Norm3Col4UV2
{
private:
	float position[3] = { 0.0f, 0.0f, 0.0f };
	float normal[3] = { 0.0f, 0.0f, 0.0f };
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float uv[2] = { 0.0f, 0.0f };

public:
	sVertexPos3Norm3Col4UV2() = default;

	sVertexPos3Norm3Col4UV2(const class vector3d& inPosition, const class vector3d& inNormal, const class vector4d& inColor, const class vector2d& inUV);
};