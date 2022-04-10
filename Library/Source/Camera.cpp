#include "./Header/Camera.h"

float Camera::targetRadius = 150.0f;
float Camera::longitude = EngineMath::radian * (-90.0f);
float Camera::latitude = EngineMath::radian * (0.0f);

EngineMath::Vector3 Camera::pos = {};
EngineMath::Vector3 Camera::target = { 0.0f, 50.0f, 0.0f };
EngineMath::Vector3 Camera::upVec = { 0.0f, 1.0f, 0.0f };

DirectX::XMMATRIX Camera::CreateCamera(const XMVECTOR& pos, const XMVECTOR& target, const XMVECTOR& upVector)
{
	using namespace DirectX;

	XMVECTOR x, y, z;
	XMMATRIX mat;

	z = target - pos;
	z = XMVector3Normalize(z);
	x = XMVector3Cross(XMVector3Normalize(upVector), z);
	x = XMVector3Normalize(x);
	y = XMVector3Cross(z, x);
	y = XMVector3Normalize(y);

	mat.r[0] = x;
	mat.r[1] = y;
	mat.r[2] = z;
	mat.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	mat = XMMatrixTranspose(mat);

	// 視点座標に-1を掛けた座標
	XMVECTOR reverseEyePosition = XMVectorNegate(pos);
	// カメラの位置からワールド原点へのベクトル（カメラ座標系）
	XMVECTOR tX = XMVector3Dot(x, reverseEyePosition); //X成分
	XMVECTOR tY = XMVector3Dot(y, reverseEyePosition); //Y成分
	XMVECTOR tZ = XMVector3Dot(z, reverseEyePosition); //Z成分
	// 一つのベクトルにまとめる
	XMVECTOR translation = XMVectorSet(tX.m128_f32[0], tY.m128_f32[1], tZ.m128_f32[2], 1.0f);
	// ビュー行列に平行移動成分を設定
	mat.r[3] = translation;

	return mat;
}

DirectX::XMMATRIX Camera::CreateCameraFix(const XMVECTOR& pos, const XMVECTOR& target, const XMVECTOR& upVector)
{
	using namespace DirectX;

	XMVECTOR x, y, z, d;
	XMMATRIX mat;

	y = XMVector3Normalize(upVector);
	d = target - pos;
	d = XMVector3Normalize(d);
	x = XMVector3Cross(y, d);
	x = XMVector3Normalize(x);
	z = XMVector3Cross(x, y);
	z = XMVector3Normalize(z);

	mat.r[0] = x;
	mat.r[1] = y;
	mat.r[2] = z;
	mat.r[3] = -pos;

	mat.r[0].m128_f32[3] = 0;
	mat.r[1].m128_f32[3] = 0;
	mat.r[2].m128_f32[3] = 0;
	mat.r[3].m128_f32[3] = 1;

	return mat;
}
