#include "./Header/Scenemanager.h"
#include "../Header/TitleScene.h"
#include "../Header/GameScene.h"
#include "../Header/GameOverScene.h"
#include "../Header/TestScene.h"
#include "./Header/Error.h"

SceneManager::SceneManager()
{
	sceneStack.push(std::make_shared<TestScene>(this));
}

void SceneManager::Loop() const
{
	sceneStack.top()->Update();
	sceneStack.top()->Draw();
}

void SceneManager::SceneChenge(const SceneManager::Scene scene, const bool stackClear)
{
	using namespace std;

	if (stackClear)
	{
		while (sceneStack.empty() == false)
		{
			sceneStack.pop();
		}
	}

	switch (scene)
	{
	case SceneManager::Scene::Title:
		sceneStack.push(make_shared<TitleScene>(this));
		break;
	case SceneManager::Scene::GameMain:
		sceneStack.push(make_shared<GameScene>(this));
		break;
	case SceneManager::Scene::GameOver:
		sceneStack.push(make_shared<GameOverScene>(this));
		break;
	case SceneManager::Scene::Test:
		sceneStack.push(make_shared<TestScene>(this));
		break;
	default:
		ErrorLog("‘¶İ‚µ‚È‚¢ƒV[ƒ“‚ªŒÄ‚Î‚ê‚Ü‚µ‚½\n");
		break;
	}
}
