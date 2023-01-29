#pragma once

class mathLibrary
{
	static uint32_t randomUInt32();

	template <typename T>
	static int32_t sign(const T Value)
	{
		return (T(0) < Value) - (Value < T(0));
	}

	static float roundTo1DecimalPlace(const float value, const bool negativeAwayFromZero = true);
	static float roundTo2DecimalPlace(const float value, const bool negativeAwayFromZero = true);
	static float roundTo3DecimalPlace(const float value, const bool negativeAwayFromZero = true);
	static double roundTo1DecimalPlace(const double value, const bool negativeAwayFromZero = true);
	static double roundTo2DecimalPlace(const double value, const bool negativeAwayFromZero = true);
	static double roundTo3DecimalPlace(const double value, const bool negativeAwayFromZero = true);
};