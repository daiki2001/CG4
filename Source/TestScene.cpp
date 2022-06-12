#include "./Header/TestScene.h"
#include "./Header/DirectXInit.h"
#include "./Header/Input.h"
#include "./Header/Camera.h"

#include "./Header/Error.h"

TestScene::TestScene(SceneChenger* sceneChenger) :
	BaseScene(sceneChenger),
	fbxLoader(FbxLoader::GetInstance()),
	background(Engine::functionError),
	fbxModel(Engine::functionError)
{
	Init();
}

TestScene::~TestScene()
{
	fbxLoader->Finalize();
}

void TestScene::Init()
{
	using namespace DirectX;
	using namespace Engine::Math;

	fbxLoader->Init();
	fbxModel = fbxLoader->LoadModelFromFile("./Resources/cube/cube.fbx");
	fbxLoader->GetModel(fbxModel)->CreateConstBuffer();
	fbxLoader->GetModel(fbxModel)->Init();

	// ‰æ‘œ‚Ì“Ç‚Ýž‚Ý
	background = draw.LoadTextrue(L"./Resources/background.png");

	Camera::targetRadius = 150.0f;
	Camera::longitude = degree * (-90.0f);
	Camera::latitude = degree * (0.0f);

	Camera::pos = {};
	Camera::target = { 0.0f, 50.0f, 0.0f };
	Camera::upVec = { 0.0f, 1.0f, 0.0f };

	fbxLoader->GetModel(fbxModel)->pos = { 0.0f, 0.0f, -10.0f };
}

void TestScene::Update()
{
	using namespace DirectX;
	using namespace Engine::Math;
	using namespace Collision;

	if (Input::IsKey(DIK_R))
	{
		Camera::targetRadius = 150.0f;
		Camera::longitude = degree * (-90.0f);
		Camera::latitude = degree * (0.0f);
	}

	if (Input::IsKey(DIK_A))
	{
		Camera::longitude -= degree;
		if (Camera::longitude <= degree * 0.0)
		{
			Camera::longitude += degree * 360;
		}
	}
	if (Input::IsKey(DIK_D))
	{
		Camera::longitude += degree;
		if (Camera::longitude >= degree * 360.0)
		{
			Camera::longitude -= degree * 360;
		}
	}
	if (Input::IsKey(DIK_W))
	{
		Camera::latitude += degree;
		if (Camera::latitude >= degree * 90.0f)
		{
			Camera::latitude = degree * 90.0f - degree;
		}
	}
	if (Input::IsKey(DIK_S))
	{
		Camera::latitude -= degree;
		if (Camera::latitude <= degree * -90.0f)
		{
			Camera::latitude = -(degree * 90.0f - degree);
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
	Camera::SetCamera(Camera::pos, Camera::target, Camera::upVec);

	fbxLoader->GetModel(fbxModel)->Update();
}

void TestScene::Draw()
{
	using namespace DirectX;
	using namespace Engine::Math;

	DirectXInit* w = DirectXInit::GetInstance();

	w->ClearScreen();
	draw.SetDrawBlendMode(BLENDMODE_ALPHA);

	// ”wŒi
	draw.DrawTextrue(
		0.0f,
		0.0f,
		(float)w->windowWidth,
		(float)w->windowHeight,
		0.0f,
		background,
		XMFLOAT2(0.0f, 0.0f)
	);

	fbxLoader->GetModel(fbxModel)->Draw();

	// ƒ‹[ƒv‚ÌI—¹ˆ—
	draw.PolygonLoopEnd();
	w->ScreenFlip();
}
