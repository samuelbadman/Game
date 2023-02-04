#pragma once

class mathLibrary
{
public:
	static uint32_t randomUInt32();

	template <typename T>
	static int32_t sign(const T Value)
	{
		return (T(0) < Value) - (Value < T(0));
	}

	static float fRoundTo1DecimalPlace(const float value, const bool negativeAwayFromZero = true);
	static float fRoundTo2DecimalPlace(const float value, const bool negativeAwayFromZero = true);
	static float fRoundTo3DecimalPlace(const float value, const bool negativeAwayFromZero = true);
	static double roundTo1DecimalPlace(const double value, const bool negativeAwayFromZero = true);
	static double roundTo2DecimalPlace(const double value, const bool negativeAwayFromZero = true);
	static double roundTo3DecimalPlace(const double value, const bool negativeAwayFromZero = true);

	static float fDegreesToRadians(const float degrees);
	static double degreesToRadians(const double degrees);
	static float fRadiansToDegrees(const float radians);
	static double radiansToDegrees(const double radians);
};