#pragma once
#include "DrawPolygon.h"
#include "SceneChenge.h"

class BaseScene
{
public:
	BaseScene() = delete;
	BaseScene(SceneChenger* sceneChenger);
	~BaseScene();

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

protected:
	SceneChenger* sceneChenger;
	DrawPolygon draw;
};
