#pragma once

class vector4d
{
public:
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	double w = 1.0;

public:
	vector4d() = default;

	template <typename T>
	vector4d(const T inX, const T inY, const T inZ, const T inW) 
		: x(static_cast<double>(inX)), y(static_cast<double>(inY)), z(static_cast<double>(inZ)), w(static_cast<double>(inW)) {}

public:
	static vector4d normalize(const vector4d& a);

public:
	std::string toString() const;
};