#include "./Header/DirectXInit.h"
#include "./Header/DrawPolygon.h"
#include "./Header/Input.h"
#include "./Math/Collision/Collision.h"
#include "./Header/SceneManager.h"

/*ウィンドウサイズ*/
const int window_width = 1280; //横幅
const int window_height = 720; //縦幅

const WCHAR title[] = L"DirectXGame"; //タイトル

const float clearColor[] = { 0.1f,0.875f,0.875f,0.0f }; //背景色

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	DirectXInit* w = DirectXInit::GetInstance();

	w->title = title;
	for (UINT i = 0; i < 4; i++)
	{
		w->clearColor[i] = clearColor[i];
	}
	w->SetWindowSize(window_width, window_height);
	w->Init();

	DrawPolygon draw(w);

	Input::Init();

	SceneManager sceneManager;

	while (true)
	{
		if (w->WindowMessage() == -1) { break; }

		/* 更新 */
		Input::Update();

		sceneManager.Loop();

		if (Input::IsKey(DIK_ESCAPE)) { break; }
	}

	w->WindowEnd();

	return 0;
}