#include "./Header/TestScene.h"
#include "./Header/Input.h"
#include "./Header/Camera.h"

const float TestScene::sphereRadius = 10.0f;
const float TestScene::planeRadius = 10.0f;
const EngineMath::Vector3 TestScene::tPos1 = { -1.0f, -1.0f, 0.0f };
const EngineMath::Vector3 TestScene::tPos2 = { 0.0f, 1.0f, 0.0f };
const EngineMath::Vector3 TestScene::tPos3 = { 1.0f, -1.0f, 0.0f };

const DirectX::XMFLOAT4 TestScene::color[3] = {
		{ 1.0f, 1.0f, 1.0f, 1.0f }, //当たって無い時の色
		{ 1.0f, 0.0f, 0.0f, 1.0f }, //当たった時の色
		{ 0.5f, 0.5f, 0.5f, 1.0f }, //当たり判定が無いやつ用
};

const float TestScene::rayLength = 100.0f;

const float TestScene::triangleAngle = -30.0f;
const float TestScene::triangleScale = 10.0f;

TestScene::TestScene(SceneChenger* sceneChenger) :
	BaseScene(sceneChenger),
	background(-1),
	sphere(-1),
	plane(-1),
	triangle(-1),
	ray(-1),
	player(nullptr),
	playerPos{},
	rayPos{},
	rayDir(0.0f, 0.0f, -1.0f),
	rayAngle{},
	rayRot{},
	rayDistance(0.0f),
	spherePos{},
	planePos{},
	planeRot{},
	planeScale(1.0f),
	planeToRayHitPos{},
	trianglePos{},
	triangleRot{},
	triangleInvRot{},
	triangleMatrix{},
	playerCollision{},
	sphereCollision{},
	planeCollision{},
	triangleCollision{},
	rayCollision{},
	isCameraMove(false),
	isRay(false)
{
	player = Player::Get();
	Init();
}

void TestScene::Init()
{
	using namespace DirectX;
	using namespace EngineMath;

	// 画像の読み込み
	background = draw.LoadTextrue(L"./Resources/background.png");

	// オブジェクトの生成
	sphere = draw.CreateSphere(sphereRadius, 16);
	player->SetModel(sphere);
	plane = draw.CreateCircle(planeRadius, 32);
	triangle = draw.CreateTriangle(
		tPos1, { 0.0f, 1.0f }, tPos2, { 0.5f, 0.0f }, tPos3, { 1.0f, 1.0f });
	ray = draw.CreateCylinder(0.125f, sphereRadius, 4);

	// タイリング
	//draw.Tiring(plane, 50.0f, 50.0f);

	Camera::targetRadius = 150.0f;
	Camera::longitude = radian * (-90.0f);
	Camera::latitude = radian * (0.0f);

	Camera::pos = {};
	Camera::target = { 0.0f, 50.0f, 0.0f };
	Camera::upVec = { 0.0f, 1.0f, 0.0f };

	playerPos = Camera::target;

	rayPos = Vector3();
	rayDir = Vector3(0.0f, 0.0f, -1.0f);
	rayAngle = Vector3(-XM_PIDIV2, 0.0f, 0.0f);
	rayRot = XMMatrixIdentity();
	rayDistance = 0.0f;

	spherePos = { 50.0f, 50.0f, 0.0f };

	planePos = { 0.0f, -50.0f, 0.0f };
	planeRot = XMMatrixRotationX(XMConvertToRadians(90.0f));
	planeScale = 100.0f;
	planeToRayHitPos = Vector3();

	trianglePos = Vector3(-50.0f, 50.0f, 0.0f);
	triangleRot = XMMatrixRotationY(XMConvertToRadians(triangleAngle));
	triangleInvRot = XMMatrixRotationY(XMConvertToRadians(180.0f + triangleAngle));
	triangleMatrix =
		XMMatrixScaling(triangleScale, triangleScale, triangleScale) *
		triangleRot *
		XMMatrixTranslation(trianglePos.x, trianglePos.y, trianglePos.z);

	playerCollision = Sphere(playerPos, sphereRadius * 2.0f);
	sphereCollision = Sphere(spherePos, sphereRadius);
	planeCollision = Plane(planeRot, planePos.Length());
	triangleCollision = Triangle(tPos1 * triangleMatrix, tPos2 * triangleMatrix, tPos3 * triangleMatrix);
	rayCollision = Ray(playerPos, rayDir * rayRot);

	isCameraMove = false;
	isRay = false;
}

void TestScene::Update()
{
	using namespace DirectX;
	using namespace EngineMath;
	using namespace Collision;

	// フラグの初期化
	playerCollision.isHit = false;
	sphereCollision.isHit = false;
	planeCollision.isHit = false;
	triangleCollision.isHit = false;
	rayCollision.isHit = false;

	if (Input::IsKeyTrigger(DIK_RETURN))
	{
		isCameraMove = !isCameraMove;
	}
	if (Input::IsKeyTrigger(DIK_SPACE))
	{
		isRay = !isRay;
	}

	if (isCameraMove)
	{
		if (Input::IsKey(DIK_LEFT))
		{
			Camera::target.x -= 1.0f;
		}
		if (Input::IsKey(DIK_RIGHT))
		{
			Camera::target.x += 1.0f;
		}
		if (Input::IsKey(DIK_UP))
		{
			Camera::target.y += 1.0f;
		}
		if (Input::IsKey(DIK_DOWN))
		{
			Camera::target.y -= 1.0f;
		}
	}
	else if (isRay)
	{
		if (Input::IsKey(DIK_LEFT))
		{
			rayAngle.y -= radian;
			if (rayAngle.y <= radian * 0.0)
			{
				rayAngle.y += radian * 360.0f;
			}
		}
		if (Input::IsKey(DIK_RIGHT))
		{
			rayAngle.y += radian;
			if (rayAngle.y >= radian * 360.0f)
			{
				rayAngle.y -= radian * 360.0f - radian;
			}
		}
		if (Input::IsKey(DIK_UP))
		{
			rayAngle.x += radian;
			if (rayAngle.x >= radian * 360.0f)
			{
				rayAngle.x -= radian * 360.0f - radian;
			}
		}
		if (Input::IsKey(DIK_DOWN))
		{
			rayAngle.x -= radian;
			if (rayAngle.x <= radian * 0.0)
			{
				rayAngle.x += radian * 360.0f;
			}
		}

		rayRot = XMMatrixRotationX(rayAngle.x) * XMMatrixRotationY(rayAngle.y);
	}
	else
	{
		if (Input::IsKey(DIK_LEFT))
		{
			playerPos.x -= 1.0f;
			Camera::target.x -= 1.0f;
		}
		if (Input::IsKey(DIK_RIGHT))
		{
			playerPos.x += 1.0f;
			Camera::target.x += 1.0f;
		}
		if (Input::IsKey(DIK_UP))
		{
			playerPos.y += 1.0f;
			Camera::target.y += 1.0f;
		}
		if (Input::IsKey(DIK_DOWN))
		{
			playerPos.y -= 1.0f;
			Camera::target.y -= 1.0f;
		}
	}

	if (Input::IsKey(DIK_A))
	{
		Camera::longitude -= radian;
		if (Camera::longitude <= radian * 0.0)
		{
			Camera::longitude += radian * 360;
		}
	}
	if (Input::IsKey(DIK_D))
	{
		Camera::longitude += radian;
		if (Camera::longitude >= radian * 360.0)
		{
			Camera::longitude -= radian * 360;
		}
	}
	if (Input::IsKey(DIK_W))
	{
		Camera::latitude += radian;
		if (Camera::latitude >= radian * 90.0f)
		{
			Camera::latitude = radian * 90.0f - radian;
		}
	}
	if (Input::IsKey(DIK_S))
	{
		Camera::latitude -= radian;
		if (Camera::latitude <= radian * -90.0f)
		{
			Camera::latitude = -(radian * 90.0f - radian);
		}
	}

	if (Input::IsKey(DIK_PGUP))
	{
		Camera::targetRadius -= 1.0f;
		if (Camera::targetRadius < 10.0f)
		{
			Camera::targetRadius = 10.0f;
		}
	}
	if (Input::IsKey(DIK_PGDN))
	{
		Camera::targetRadius += 1.0f;
	}

	Camera::pos.x = cosf(Camera::longitude) * cosf(Camera::latitude);
	Camera::pos.y = sinf(Camera::latitude);
	Camera::pos.z = sinf(Camera::longitude) * cosf(Camera::latitude);
	Camera::pos *= Camera::targetRadius;
	Camera::pos += Camera::target;
	draw.SetCamera(Camera::pos, Camera::target, Camera::upVec);

	playerCollision.center = playerPos;

	if (isRay)
	{
		rayCollision.start = playerPos;
		rayCollision.dir = rayDir * rayRot;
		rayPos = playerPos /*+ (rayDir * 2.0f * (rayLength + sphereRadius)) * rayRot*/;

		if (IsRayToSphereCollision(rayCollision, sphereCollision, &rayDistance))
		{
			if (rayDistance < sphereRadius * rayLength)
			{
				rayCollision.isHit = true;
				sphereCollision.isHit = true;
			}
		}
		if (IsRayToPlaneCollision(rayCollision, planeCollision, &rayDistance, &planeToRayHitPos))
		{
			if ((rayDistance < sphereRadius * rayLength) && ((planeToRayHitPos - planePos).Length() < planeRadius * planeScale))
			{
				rayCollision.isHit = true;
				planeCollision.isHit = true;
			}
		}
		if (IsRayToTriangleCollision(rayCollision, triangleCollision, &rayDistance))
		{
			if (rayDistance < sphereRadius * rayLength)
			{
				rayCollision.isHit = true;
				triangleCollision.isHit = true;
			}
		}
	}
	else
	{
		if (IsSphereToSphereCollision(playerCollision, sphereCollision))
		{
			playerCollision.isHit = true;
			sphereCollision.isHit = true;
		}
		if (IsSphereToPlaneCollision(playerCollision, planeCollision))
		{
			playerCollision.isHit = true;
			planeCollision.isHit = true;
		}
		if (IsSphereToTriangleCollision(playerCollision, triangleCollision))
		{
			playerCollision.isHit = true;
			triangleCollision.isHit = true;
		}
	}
}

void TestScene::Draw()
{
	using namespace DirectX;
	using namespace EngineMath;

	DirectXInit* w = DirectXInit::GetInstance();

	w->ClearScreen();
	draw.SetDrawBlendMode(BLENDMODE_ALPHA);

	// 背景
	draw.DrawTextrue(
		0.0f,
		0.0f,
		(float)w->windowWidth,
		(float)w->windowHeight,
		0.0f,
		background,
		XMFLOAT2(0.0f, 0.0f)
	);

	if (isRay)
	{
		// プレイヤーの描画
		draw.Draw(
			sphere,
			playerPos,
			XMMatrixIdentity(),
			scale_xyz(0.5f),
			color[2]
		);
		// レイの描画
		draw.Draw(
			ray,
			rayPos,
			rayRot,
			XMFLOAT3(1.0f, 1.0f, rayLength),
			color[rayCollision.isHit]
		);
	}
	else
	{
		// プレイヤーの描画
		draw.Draw(
			sphere,
			playerPos,
			XMMatrixIdentity(),
			scale_xyz(2.0f),
			color[playerCollision.isHit]
		);
	}
	// 球体の描画
	draw.Draw(
		sphere,
		spherePos,
		XMMatrixIdentity(),
		scale_xyz(1.0f),
		color[sphereCollision.isHit]
	);
	// 平面の描画
	draw.Draw(
		plane,
		planePos,
		planeRot,
		scale_xyz(planeScale),
		color[planeCollision.isHit]
	);
	draw.Draw(
		plane,
		planePos,
		Inverse(planeRot),
		scale_xyz(planeScale),
		color[planeCollision.isHit]
	);
	// 三角形の描画
	draw.Draw(
		triangle,
		trianglePos,
		triangleRot,
		scale_xyz(triangleScale),
		color[triangleCollision.isHit]
	);
	// 三角形の描画
	draw.Draw(
		triangle,
		trianglePos,
		triangleInvRot,
		scale_xyz(triangleScale),
		color[triangleCollision.isHit]
	);

	if (isRay)
	{
		draw.DrawString(0.0f, 0.0f, 1.5f, { 0.0f, 0.0f, 0.0f, 1.0f }, "collision:ray");
		if (isCameraMove)
		{
			draw.DrawString(0.0f, 32.0f, 1.5f, { 0.0f, 0.0f, 0.0f, 1.0f }, "Move:camera translation");
		}
		else
		{
			draw.DrawString(0.0f, 32.0f, 1.5f, { 0.0f, 0.0f, 0.0f, 1.0f }, "Move:ray");
		}
	}
	else
	{
		draw.DrawString(0.0f, 0.0f, 1.5f, { 0.0f, 0.0f, 0.0f, 1.0f }, "collision:object");
		if (isCameraMove)
		{
			draw.DrawString(0.0f, 32.0f, 1.5f, { 0.0f, 0.0f, 0.0f, 1.0f }, "Move:camera translation");
		}
		else
		{
			draw.DrawString(0.0f, 32.0f, 1.5f, { 0.0f, 0.0f, 0.0f, 1.0f }, "Move:object");
		}
	}

	// ループの終了処理
	draw.PolygonLoopEnd();
	w->ScreenFlip();
}
