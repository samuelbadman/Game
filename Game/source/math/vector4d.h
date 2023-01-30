#pragma once

class vector4d
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

public:
	vector4d() = default;

	template <typename T>
	vector4d(const T inX, const T inY, const T inZ, const T inW) 
		: x(static_cast<float>(inX)), y(static_cast<float>(inY)), z(static_cast<float>(inZ)), w(static_cast<float>(inW)) {}

public:
	static vector4d normalize(const vector4d& a);

public:
	std::string toString() const;
};