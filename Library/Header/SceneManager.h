#pragma once
#include "SceneChenge.h"
#include "BaseScene.h"
#include <memory>
#include <stack>

class SceneManager : public SceneChenger
{
public: // ƒƒ“ƒoŠÖ”
	SceneManager();
	~SceneManager() {}

	void Loop() const;
	void SceneChenge(const SceneChenger::Scene scene, const bool stackClear) override;

private: // ƒƒ“ƒo•Ï”
	std::stack<std::shared_ptr<BaseScene>> sceneStack;
};
