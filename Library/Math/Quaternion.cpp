#include "Quaternion.h"
#include <math.h>

namespace Engine
{
namespace Math
{
Quaternion quaternion(float x, float y, float z, float w)
{
	Quaternion result = { x, y, z, w };
	return result;
}

Quaternion quaternion(const Vector3& v, float angle)
{
	float s = sinf(angle / 2.0f);
	return quaternion(s * v.x, s * v.y, s * v.z, cosf(angle / 2.0f));
}

float dot(const Quaternion& q1, const Quaternion& q2)
{
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

float length(const Quaternion& q)
{
	return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

Quaternion normalize(const Quaternion& q)
{
	Quaternion result = q;
	float len = length(result);
	if (len != 0)
	{
		result.x /= len;
		result.y /= len;
		result.z /= len;
		result.w /= len;
	}
	return result;
}

Quaternion conjugate(const Quaternion& q)
{
	Quaternion result = { -q.x, -q.y, -q.z, q.w };
	return result;
}

Quaternion operator+(const Quaternion& q)
{
	return q;
}

Quaternion operator-(const Quaternion& q)
{
	Quaternion result = { -q.x, -q.y, -q.z, -q.w };
	return result;
}

Quaternion& operator+=(Quaternion& q1, const Quaternion& q2)
{
	q1.x += q2.x;
	q1.y += q2.y;
	q1.z += q2.z;
	q1.w += q2.w;
	return q1;
}

Quaternion& operator-=(Quaternion& q1, const Quaternion& q2)
{
	q1.x -= q2.x;
	q1.y -= q2.y;
	q1.z -= q2.z;
	q1.w -= q2.w;
	return q1;
}

Quaternion& operator*=(Quaternion& q1, const Quaternion& q2)
{
	Quaternion result =
	{
		 q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x,
		-q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y,
		 q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z,
		-q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w,
	};
	q1 = result;
	return q1;
}

Quaternion& operator*=(Quaternion& q, float s)
{
	q.x *= s;
	q.y *= s;
	q.z *= s;
	q.w *= s;
	return q;
}

Quaternion& operator/=(Quaternion& q, float s)
{
	q.x /= s;
	q.y /= s;
	q.z /= s;
	q.w /= s;
	return q;
}

Quaternion operator+(const Quaternion& q1, const Quaternion& q2)
{
	Quaternion result = q1;
	result.x += q2.x;
	result.y += q2.y;
	result.z += q2.z;
	result.w += q2.w;
	return result;
}

Quaternion operator-(const Quaternion& q1, const Quaternion& q2)
{
	Quaternion result = q1;
	result.x -= q2.x;
	result.y -= q2.y;
	result.z -= q2.z;
	result.w -= q2.w;
	return result;
}

Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
	Quaternion result =
	{
		 q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x,
		-q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y,
		 q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z,
		-q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w,
	};
	return result;
}

Quaternion operator*(const Quaternion& q, float s)
{
	Quaternion result = q;
	result.x *= s;
	result.y *= s;
	result.z *= s;
	result.w *= s;
	return result;
}

Quaternion operator*(float s, const Quaternion& q)
{
	Quaternion result = q;
	result.x *= s;
	result.y *= s;
	result.z *= s;
	result.w *= s;
	return result;
}

Quaternion operator/(const Quaternion& q, float s)
{
	Quaternion result = q;
	result.x /= s;
	result.y /= s;
	result.z /= s;
	result.w /= s;
	return result;
}

Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float t)
{
	float dot_angle = dot(q1, q2);
	Quaternion t2 = q2;
	if (dot_angle < 0.0f)
	{
		dot_angle = -dot_angle;
		t2 = -q2;
	}
	float k0 = 1.0f - t;
	float k1 = t;
	if ((1.0f - dot_angle) > 0.001f)
	{
		float theta = acosf(dot_angle);
		k0 = sinf(theta * k0) / sinf(theta);
		k1 = sinf(theta * k1) / sinf(theta);
	}
	return q1 * k0 + t2 * k1;
}

Quaternion lerp(const Quaternion& q1, const Quaternion& q2, float t)
{
	float dot_angle = dot(q1, q2);
	Quaternion t2 = q2;
	if (dot_angle < 0.0f)
	{
		dot_angle = -dot_angle;
		t2 = -q2;
	}
	float k0 = 1.0f - t;
	float k1 = t;
	return q1 * k0 + t2 * k1;
}

Matrix4 rotate(const Quaternion& q)
{
	float xx = q.x * q.x * 2.0f;
	float yy = q.y * q.y * 2.0f;
	float zz = q.z * q.z * 2.0f;
	float xy = q.x * q.y * 2.0f;
	float xz = q.x * q.z * 2.0f;
	float yz = q.y * q.z * 2.0f;
	float wx = q.w * q.x * 2.0f;
	float wy = q.w * q.y * 2.0f;
	float wz = q.w * q.z * 2.0f;
	Matrix4 result = {
		1.0f - yy - zz, xy + wz,        xz - wy,        0.0f,
		xy - wz,        1.0f - xx - zz, yz + wx,        0.0f,
		xz + wy,        yz - wx,        1.0f - xx - yy, 0.0f,
		0.0f,           0.0f,           0.0f,           1.0f
	};
	return result;
}

Quaternion quaternion(const Matrix4& m)
{
	Quaternion result = {};
	float tr = m.r[0].m128_f32[0] + m.r[1].m128_f32[1] + m.r[2].m128_f32[2] + m.r[3].m128_f32[3];

	if (tr >= 1.0f)
	{
		float fourD = 2.0f * sqrtf(tr);
		result.x = (m.r[1].m128_f32[2] - m.r[2].m128_f32[1]) / fourD;
		result.y = (m.r[2].m128_f32[0] - m.r[0].m128_f32[2]) / fourD;
		result.z = (m.r[0].m128_f32[1] - m.r[1].m128_f32[0]) / fourD;
		result.w = fourD / 4.0f;
		return result;
	}

	int i = 0;
	if (m.r[0].m128_f32[0] <= m.r[1].m128_f32[1])
	{
		i = 1;
	}
	if (m.r[2].m128_f32[2] > m.r[i].m128_f32[i])
	{
		i = 2;
	}

	int j = (i + 1) % 3;
	int k = (j + 1) % 3;
	tr = m.r[i].m128_f32[i] - m.r[j].m128_f32[j] - m.r[k].m128_f32[k] + 1.0f;
	float fourD = 2.0f * sqrtf(tr);
	float qa[4] = {};
	qa[i] = fourD / 4.0f;
	qa[j] = (m.r[j].m128_f32[i] + m.r[i].m128_f32[j]) / fourD;
	qa[k] = (m.r[k].m128_f32[i] + m.r[i].m128_f32[k]) / fourD;
	qa[3] = (m.r[j].m128_f32[k] - m.r[k].m128_f32[j]) / fourD;
	result = { qa[0], qa[1], qa[2], qa[3] };
	return result;
}

Quaternion quaternion(const Vector3& v, const Quaternion& q)
{
	Quaternion qq = conjugate(q);
	Quaternion qv = quaternion(v.x, v.y, v.z, 0.0f); //ベクトルvをクォータニオン化する

	return q * qv * qq; //ベクトルを回転したクォータニオンを返す
}

Vector3 getAxis(const Quaternion& q)
{
	Vector3 result;
	float len = length(q);

	result.x = q.x / len;
	result.y = q.y / len;
	result.z = q.z / len;

	return result;
}
} //Math
} //Engine
