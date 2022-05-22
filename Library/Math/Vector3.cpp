#include "Vector3.h"
#include <cmath>

namespace Engine
{
namespace Math
{
Vector3::Vector3() :
	DirectX::XMFLOAT3{ 0, 0, 0 }
{
}

Vector3::Vector3(float x, float y, float z) :
	DirectX::XMFLOAT3{ x, y, z }
{
}

Vector3::Vector3(XMFLOAT3 v) :
	DirectX::XMFLOAT3(v)
{
}

float Vector3::Length() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}

float Vector3::LengthSquared() const
{
	float len = Length();
	return len * len;
}

Vector3 Vector3::Normalize() const
{
	Vector3 result = *this;
	float len = Length();
	if (len != 0)
	{
		return result /= len;
	}

	return result;
}

float Vector3::Dot(const Vector3& v) const
{
	return (this->x * v.x) + (this->y * v.y) + (this->z * v.z);
}

Vector3 Vector3::Cross(const Vector3& v) const
{
	return Vector3(
		(this->y * v.z) - (this->z * v.y),
		(this->z * v.x) - (this->x * v.z),
		(this->x * v.y) - (this->y * v.x)
	);
}

Vector3 Vector3::operator+() const
{
	return *this;
}

Vector3 Vector3::operator-() const
{
	return Vector3(-this->x, -this->y, -this->z);
}

Vector3& Vector3::operator+=(const Vector3& v)
{
	this->x += v.x;
	this->y += v.y;
	this->z += v.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& v)
{
	this->x -= v.x;
	this->y -= v.y;
	this->z -= v.z;
	return *this;
}

Vector3& Vector3::operator*=(float s)
{
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

Vector3& Vector3::operator/=(float s)
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

bool Vector3::operator==(const Vector3& v)
{
	return this->x == v.x && this->y == v.y && this->z == v.z;
}

bool Vector3::operator!=(const Vector3& v)
{
	return !(*this == v);
}

const Vector3 operator+(const Vector3& v1, const Vector3& v2)
{
	Vector3 temp(v1);
	return temp += v2;
}

const Vector3 operator-(const Vector3& v1, const Vector3& v2)
{
	Vector3 temp(v1);
	return temp -= v2;
}

const Vector3 operator*(const Vector3& v, float s)
{
	Vector3 temp(v);
	return temp *= s;
}

const Vector3 operator*(float s, const Vector3& v)
{
	return v * s;
}

const Vector3 operator/(const Vector3& v, float s)
{
	Vector3 temp(v);
	return temp /= s;
}
} //Math
} //Engine
