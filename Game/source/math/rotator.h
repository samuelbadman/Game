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
	rotator(const T inPitch, const T inYaw, const T inRoll) 
		: pitch(static_cast<float>(inPitch)), yaw(static_cast<float>(inYaw)), roll(static_cast<float>(inRoll)) {}

public:
	quaternion toQuaternion() const;
};