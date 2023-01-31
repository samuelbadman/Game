#pragma once

#include "quaternion.h"

class rotator
{
public:
	// Expressed in degrees
	double pitch = 0.0;
	// Expressed in degrees
	double yaw = 0.0;
	// Expressed in degrees
	double roll = 0.0;

public:
	rotator() = default;

	template <typename T>
	rotator(const T inPitchDegrees, const T inYawDegrees, const T inRollDegrees) 
		: pitch(static_cast<double>(inPitchDegrees)), yaw(static_cast<double>(inYawDegrees)), roll(static_cast<double>(inRollDegrees)) {}

public:
	quaternion toQuaternion() const;
};