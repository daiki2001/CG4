#pragma once
#include <DirectXMath.h>
#include "Vector3.h"

namespace Engine
{
namespace Math
{
// DirectX::XMMATRIX 型を Matrix4 型として扱う ※互換性の確保&拡張のため
using Matrix4 = DirectX::XMMATRIX;

// 単位行列を求める
Matrix4 Identity();

// 逆行列を求める
Matrix4 Inverse(const Matrix4& m);

// 拡大縮小行列の設定
Matrix4 scale(const Vector3& s);

// 回転行列の設定
Matrix4 rotateX(float angle);
Matrix4 rotateY(float angle);
Matrix4 rotateZ(float angle);

// 平行移動行列の作成
Matrix4 translate(const Vector3& t);

// 座標変換（ベクトルと行列の掛け算をする）
Vector3 transform(const Vector3& v, const Matrix4& m);

// 2項演算子オーバーロード
Vector3 operator * (const Vector3& v, const Matrix4& m);
} //Math
} //Engine
