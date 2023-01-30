#pragma once

class vector3d
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

public:
	vector3d() = default;

	template <typename T>
	vector3d(const T inX, const T inY, const T inZ) 
		: x(static_cast<float>(inX)), y(static_cast<float>(inY)), z(static_cast<float>(inZ)) {}

	void operator=(const vector3d& rhs);

public:
	vector3d operator-(const vector3d& rhs) const;
	vector3d operator+(const vector3d& rhs) const;
	friend vector3d operator*(const float lhs, const vector3d& rhs);

public:
	static vector3d crossProduct(const vector3d& a, const vector3d& b);
	static float dotProduct(const vector3d& a, const vector3d& b);
	static vector3d normalize(const vector3d& a);

public:
	void normalizeInPlace();
	std::string toString() const;
};