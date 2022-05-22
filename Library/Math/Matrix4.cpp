#include "Matrix4.h"
#include "Vector3.h"
#include <math.h>
#include <Windows.h>

namespace Engine
{
namespace Math
{
// 単位行列を求める
Matrix4 Identity()
{
	static const Matrix4 result
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return result;
}

Matrix4 Inverse(const Matrix4& m)
{
	static Matrix4 result{};
	static float determinant = 0.0f; //行列式

	determinant = 0.0f;
	determinant += m.r[0].m128_f32[0] * (
		m.r[1].m128_f32[1] * (m.r[2].m128_f32[2] * m.r[3].m128_f32[3] - m.r[3].m128_f32[2] * m.r[2].m128_f32[3]) -
		m.r[2].m128_f32[1] * (m.r[1].m128_f32[2] * m.r[3].m128_f32[3] - m.r[3].m128_f32[2] * m.r[1].m128_f32[3]) +
		m.r[3].m128_f32[1] * (m.r[1].m128_f32[2] * m.r[2].m128_f32[3] - m.r[2].m128_f32[2] * m.r[1].m128_f32[3]));
	determinant -= m.r[0].m128_f32[1] * (
		m.r[1].m128_f32[0] * (m.r[2].m128_f32[2] * m.r[3].m128_f32[3] - m.r[3].m128_f32[2] * m.r[2].m128_f32[3]) -
		m.r[2].m128_f32[0] * (m.r[1].m128_f32[2] * m.r[3].m128_f32[3] - m.r[3].m128_f32[2] * m.r[1].m128_f32[3]) +
		m.r[3].m128_f32[0] * (m.r[1].m128_f32[2] * m.r[2].m128_f32[3] - m.r[2].m128_f32[2] * m.r[1].m128_f32[3]));
	determinant += m.r[0].m128_f32[2] * (
		m.r[1].m128_f32[0] * (m.r[2].m128_f32[1] * m.r[3].m128_f32[3] - m.r[3].m128_f32[1] * m.r[2].m128_f32[3]) -
		m.r[2].m128_f32[0] * (m.r[1].m128_f32[1] * m.r[3].m128_f32[3] - m.r[3].m128_f32[1] * m.r[1].m128_f32[3]) +
		m.r[3].m128_f32[0] * (m.r[1].m128_f32[1] * m.r[2].m128_f32[3] - m.r[2].m128_f32[1] * m.r[1].m128_f32[3]));
	determinant -= m.r[0].m128_f32[3] * (
		m.r[1].m128_f32[0] * (m.r[2].m128_f32[1] * m.r[3].m128_f32[2] - m.r[3].m128_f32[1] * m.r[2].m128_f32[2]) -
		m.r[2].m128_f32[0] * (m.r[1].m128_f32[1] * m.r[3].m128_f32[2] - m.r[3].m128_f32[1] * m.r[1].m128_f32[2]) +
		m.r[3].m128_f32[0] * (m.r[1].m128_f32[1] * m.r[2].m128_f32[2] - m.r[2].m128_f32[1] * m.r[1].m128_f32[2]));

	if (determinant == 0)
	{
#ifdef _DEBUG
		OutputDebugStringA("逆行列がありません\n");
#endif // _DEBUG

		result = {
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f
		};
	}
	else
	{
		for (size_t i = 0; i < 4; i++)
		{
			static float symbol = 0.0f;
			static size_t a = 0, b = 0, c = 0;
			static size_t d = 0, e = 0, f = 0;

			a = (i < 1) ? 1 : 0;
			b = (i < 2) ? 2 : 1;
			c = (i < 3) ? 3 : 2;

			for (size_t j = 0; j < 4; j++)
			{
				if ((i + j) % 2)
				{
					symbol = -1.0f;
				}
				else
				{
					symbol = +1.0f;
				}

				d = (j < 1) ? 1 : 0;
				e = (j < 2) ? 2 : 1;
				f = (j < 3) ? 3 : 2;

				result.r[j].m128_f32[i] = (1.0f / determinant) * symbol * (
					m.r[a].m128_f32[d] * (m.r[b].m128_f32[e] * m.r[c].m128_f32[f] - m.r[c].m128_f32[e] * m.r[b].m128_f32[f]) -
					m.r[b].m128_f32[d] * (m.r[a].m128_f32[e] * m.r[c].m128_f32[f] - m.r[c].m128_f32[e] * m.r[a].m128_f32[f]) +
					m.r[c].m128_f32[d] * (m.r[a].m128_f32[e] * m.r[b].m128_f32[f] - m.r[b].m128_f32[e] * m.r[a].m128_f32[f]));
			}
		}
	}

	return result;
}

// 拡大縮小行列を求める
Matrix4 scale(const Vector3& s)
{
	Matrix4 result
	{
		+s.x, 0.0f, 0.0f, 0.0f,
		0.0f, +s.y, 0.0f, 0.0f,
		0.0f, 0.0f, +s.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return result;
}

// X軸まわりの回転行列を求める
Matrix4 rotateX(float angle)
{
	float sinX = sinf(angle);
	float cosX = cosf(angle);

	Matrix4 result
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosX, sinX, 0.0f,
		0.0f,-sinX, cosX, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return result;
}

// Y軸まわりの回転行列を求める
Matrix4 rotateY(float angle)
{
	float sinY = sinf(angle);
	float cosY = cosf(angle);

	Matrix4 result
	{
		cosY, 0.0f,-sinY, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		sinY, 0.0f, cosY, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return result;
}

// Z軸まわりの回転行列を求める
Matrix4 rotateZ(float angle)
{
	float sinZ = sinf(angle);
	float cosZ = cosf(angle);

	Matrix4 result
	{
		 cosZ, sinZ, 0.0f, 0.0f,
		-sinZ, cosZ, 0.0f, 0.0f,
		 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f, 0.0f, 0.0f, 1.0f
	};

	return result;
}

// 平行移動行列を求める
Matrix4 translate(const Vector3& t)
{
	Matrix4 result
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		+t.x, +t.y, +t.z, 1.0f
	};

	return result;
}

// 座標変換（ベクトルと行列の掛け算）を行う
Vector3 transform(const Vector3& v, const Matrix4& m)
{
	float w = v.x * m.r[0].m128_f32[3] + v.y * m.r[1].m128_f32[3] + v.z * m.r[2].m128_f32[3] + m.r[3].m128_f32[3];

	Vector3 result
	{
		(v.x * m.r[0].m128_f32[0] + v.y * m.r[1].m128_f32[0] + v.z * m.r[2].m128_f32[0] + m.r[3].m128_f32[0]) / w,
		(v.x * m.r[0].m128_f32[1] + v.y * m.r[1].m128_f32[1] + v.z * m.r[2].m128_f32[1] + m.r[3].m128_f32[1]) / w,
		(v.x * m.r[0].m128_f32[2] + v.y * m.r[1].m128_f32[2] + v.z * m.r[2].m128_f32[2] + m.r[3].m128_f32[2]) / w
	};

	return result;
}

// 2項演算子*オーバーロード（ベクトルと行列の積）
Vector3 operator * (const Vector3& v, const Matrix4& m)
{
	return transform(v, m);
}
} //Math
} //Engine
