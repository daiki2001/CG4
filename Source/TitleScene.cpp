#include "../Header/TitleScene.h"
#include "./Header/Input.h"

TitleScene::TitleScene(SceneChenger* sceneChenger) :
	BaseScene(sceneChenger),
	titleFont(-1),
	spaceFont(-1)
{
	Init();
}

void TitleScene::Init()
{
	/*‰æ‘œ‚Ì“Ç‚Ýž‚Ý*/
	titleFont = draw.LoadTextrue(L"./Resources/title.png");
	spaceFont = draw.LoadTextrue(L"./Resources/space.png");
}

void TitleScene::Update()
{
	if (Input::IsKeyTrigger(DIK_SPACE))
	{
		sceneChenger->SceneChenge(SceneChenger::Scene::GameMain, true);
	}
}

void TitleScene::Draw()
{
	DirectXInit* w = DirectXInit::GetInstance();

	w->ClearScreen();
	draw.SetDrawBlendMode(BLENDMODE_ALPHA);

	draw.DrawTextrue(w->windowWidth / 2.0f, (w->windowHeight / 2.0f) - 150.0f, 994.0f, 114.0f, 0.0f, titleFont, { 0.5f, 0.5f });
	draw.DrawTextrue(w->windowWidth / 2.0f, (w->windowHeight / 2.0f) + 100.0f, 576.0f, 64.0f, 0.0f, spaceFont, { 0.5f, 0.5f });

	// ƒ‹[ƒv‚ÌI—¹ˆ—
	draw.PolygonLoopEnd();
	w->ScreenFlip();
}
