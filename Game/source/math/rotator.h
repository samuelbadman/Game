#pragma once

#include "quaternion.h"

class rotator
{
public:
	// Expressed in degrees
	float pitch = 0.f;
	// Expressed in degrees
	float yaw = 0.f;
	// Expressed in degrees
	float roll = 0.f;

public:
	rotator() = default;

	template <typename T>
	rotator(const T inPitchDegrees, const T inYawDegrees, const T inRollDegrees) 
		: pitch(static_cast<float>(inPitchDegrees)), yaw(static_cast<float>(inYawDegrees)), roll(static_cast<float>(inRollDegrees)) {}

public:
	quaternion toQuaternion() const;
};