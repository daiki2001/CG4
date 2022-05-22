#pragma once
#include <DirectXMath.h>

namespace Engine
{
namespace Math
{
class Vector3 : public DirectX::XMFLOAT3
{
public:
	// コンストラクタ
	Vector3();                          //零ベクトルとして生成
	Vector3(float x, float y, float z); //x成分、y成分、z成分を指定して生成
	Vector3(XMFLOAT3 v);                //DirectX::XMFLOAT3と互換性を持たせるため

	// メンバ関数
	float Length() const;                  //ノルム(長さ)を求める
	float LengthSquared() const;           //ノルム(長さ)を求める
	Vector3 Normalize() const;             //正規化する
	float Dot(const Vector3& v) const;     //内積を求める
	Vector3 Cross(const Vector3& v) const; //外積を求める

	// 単項演算子オーバーロード
	Vector3 operator+() const;
	Vector3 operator-() const;

	// 代入演算子オーバーロード
	Vector3& operator+=(const Vector3& v);
	Vector3& operator-=(const Vector3& v);
	Vector3& operator*=(float s);
	Vector3& operator/=(float s);

	// 比較演算子オーバーロード
	bool operator==(const Vector3& v);
	bool operator!=(const Vector3& v);

	inline static Vector3 Zero() { return { 0.0f, 0.0f, 0.0f }; }
};

// 2項演算子オーバーロード
// ※いろんな引数(引数の型と順序)のパターンに対応するため、以下のように準備している
const Vector3 operator+(const Vector3& v1, const Vector3& v2);
const Vector3 operator-(const Vector3& v1, const Vector3& v2);
const Vector3 operator*(const Vector3& v, float s);
const Vector3 operator*(float s, const Vector3& v);
const Vector3 operator/(const Vector3& v, float s);
} //Math
} //Engine
