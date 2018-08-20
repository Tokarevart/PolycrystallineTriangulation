#pragma once
#include <cmath>

enum AngleUnit
{
	Radian,
	Degree
};

class Vector2
{
private:
	double _coors[2];

public:
	static const double DotProduct(const Vector2& vec0, const Vector2& vec1);
	static const double CrossProductMagnitude(const Vector2& vec0, const Vector2& vec1);
	static const double Cos(const Vector2& vec0, const Vector2& vec1);
	//static Vector2 Project(Vector2& point, Vector2& linePoint0, Vector2& linePoint1);
	//static const bool Project(Vector2& out, Vector2& point, Vector2& linePoint0, Vector2& linePoint1);
	static Vector2 LinesIntersection(const Vector2& line0p0, const Vector2& line0p1, const Vector2& line1p0, const Vector2& line1p1);
	static const bool SegmentsIntersection(const Vector2& segm0p0, const Vector2& segm0p1, const Vector2& segm1p0, const Vector2& segm1p1);
	static const bool SegmentsIntersection(Vector2& out, const Vector2& segm0p0, const Vector2& segm0p1, const Vector2& segm1p0, const Vector2& segm1p1);

	const double Magnitude() const;
	const double SqrMagnitude() const;

	Vector2& Normalize();
	Vector2& Rotate(const double& angle, const AngleUnit& unit);
	Vector2& Mirror(const Vector2& vec);
	Vector2& Project(const Vector2& vec);
	//Vector2& Project(const Vector2& linePoint0, const Vector2& linePoint1);

	Vector2& operator=(const Vector2& vec);
	double& operator[](const int& axisIndex);
	Vector2 operator+() const;
	Vector2 operator-() const;
	Vector2 operator+(const Vector2& right) const;
	Vector2 operator-(const Vector2& right) const;
	Vector2& operator+=(const Vector2& right);
	Vector2& operator-=(const Vector2& right);
	Vector2& operator*=(const double& scalar);
	friend const Vector2 operator*(const Vector2& vec, const double& scalar);
	friend const Vector2 operator*(const double& scalar, const Vector2& vec);
	friend const Vector2 operator/(const Vector2& vec, const double& scalar);

	Vector2();
	Vector2(const Vector2& vec);
	Vector2(const double& coor0, const double& coor1);
	~Vector2();
};