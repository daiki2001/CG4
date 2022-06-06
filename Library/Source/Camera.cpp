#include "./Header/Camera.h"
#include "./Header/DirectXInit.h"
#include "./Header/Error.h"

const size_t Camera::MAIN_CAMERA = 0;

float Camera::targetRadius = 150.0f;
float Camera::longitude = Engine::Math::degree * (-90.0f);
float Camera::latitude = Engine::Math::degree * (0.0f);

Engine::Math::Vector3 Camera::pos = {};
Engine::Math::Vector3 Camera::target = { 0.0f, 50.0f, 0.0f };
Engine::Math::Vector3 Camera::upVec = { 0.0f, 1.0f, 0.0f };

Engine::Math::Matrix4 Camera::matProjection[2] = {};
std::vector<Engine::Math::Matrix4> Camera::matView = {};
size_t Camera::cameraNo = Camera::MAIN_CAMERA;

Camera* Camera::Get()
{
	static Camera ins = {};
	return &ins;
}

Camera::Camera()
{
	Init();
}

void Camera::Init()
{
	using namespace DirectX;
	using namespace Engine::Math;

	matProjection[Projection::ORTHOGRAPHIC] =
		XMMatrixOrthographicOffCenterLH(
			0.0f,
			DirectXInit::GetInstance()->windowWidth,
			DirectXInit::GetInstance()->windowHeight,
			0.0f,
			0.0f,
			1.0f
		);
	matProjection[Projection::PERSPECTIVE] =
		XMMatrixPerspectiveFovLH(
			degree * 60.0f,
			static_cast<float>(DirectXInit::GetInstance()->windowWidth) /
			static_cast<float>(DirectXInit::GetInstance()->windowHeight),
			0.1f,
			1000.0f
		);

	if (matView.size() <= 0)
	{
		matView.push_back(CreateCamera(
			XMLoadFloat3(&pos),
			XMLoadFloat3(&target),
			XMLoadFloat3(&upVec)
		));
	}
	else
	{
		matView[0] = CreateCamera(
			XMLoadFloat3(&pos),
			XMLoadFloat3(&target),
			XMLoadFloat3(&upVec)
		);
	}
}

DirectX::XMMATRIX Camera::CreateCamera(const XMVECTOR& pos, const XMVECTOR& target, const XMVECTOR& upVector)
{
	using namespace DirectX;

	XMVECTOR x, y, z;
	XMMATRIX mat = {};

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
	XMMATRIX mat = {};

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

int Camera::ChangeCamera(const size_t& cameraNo)
{
	if (cameraNo < 0 || cameraNo >= matView.size())
	{
		return Engine::functionError;
	}

	Camera::cameraNo = cameraNo;
	return 0;
}

void Camera::SetCamera(const Engine::Math::Vector3& cameraPos, const Engine::Math::Vector3& cameraTarget, const Engine::Math::Vector3& upVector)
{
	using namespace DirectX;

	matView[cameraNo] = CreateCamera(
		XMLoadFloat3(&cameraPos),
		XMLoadFloat3(&cameraTarget),
		XMLoadFloat3(&upVector)
	);
}
