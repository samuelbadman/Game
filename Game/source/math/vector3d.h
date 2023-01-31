#pragma once

class vector3d
{
public:
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

public:
	vector3d() = default;

	template <typename T>
	vector3d(const T inX, const T inY, const T inZ) 
		: x(static_cast<double>(inX)), y(static_cast<double>(inY)), z(static_cast<double>(inZ)) {}

	void operator=(const vector3d& rhs);

public:
	vector3d operator-(const vector3d& rhs) const;
	vector3d operator+(const vector3d& rhs) const;
	friend vector3d operator*(const double lhs, const vector3d& rhs);

public:
	static vector3d crossProduct(const vector3d& a, const vector3d& b);
	static double dotProduct(const vector3d& a, const vector3d& b);
	static vector3d normalize(const vector3d& a);

public:
	void normalizeInPlace();
	std::string toString() const;
};