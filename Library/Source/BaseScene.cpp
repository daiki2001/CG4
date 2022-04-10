#include "./Header/BaseScene.h"
#include "./Header/DirectXInit.h"

BaseScene::BaseScene(SceneChenger* sceneChenger) :
	sceneChenger(sceneChenger),
	draw(DirectXInit::GetInstance())
{
}

BaseScene::~BaseScene()
{
}
