#include "./Header/Win32API.h"

HWND Win32API::hWnd{};
WNDCLASSEX Win32API::wndClass{};
#ifdef UNICODE
LPCWSTR Win32API::className = L"DirectX";
LPCWSTR Win32API::title = L"DirectXGame";
#else
LPCSTR Win32API::className = "DirectX";
LPCSTR Win32API::title = "DirectXGame";
#endif // UNICODE

LRESULT Win32API::WindowProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wparam, lparam);
}

Win32API::Win32API() :
	windowWidth(1),
	windowHeight(1),
	windowInit(false),
	MessageCount(0)
{
}

BOOL WINAPI Win32API::WindowInit()
{
	if (windowInit == true)
	{
		return TRUE;
	}
	else
	{
		windowInit = true;
	}

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = (WNDPROC)WindowProc;
	wndClass.lpszClassName = className;
	wndClass.hInstance = GetModuleHandle(nullptr);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RECT windowSize = { 0, 0, windowWidth, windowHeight };
	// ウィンドウクラスをOSに登録
	RegisterClassEx(&wndClass);
	// ウィンドウサイズ
	if (AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, false) == false)
	{
		return false;
	}

	hWnd = CreateWindow(
		wndClass.lpszClassName, //クラス名
		title,                  //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,    //標準的なウィンドウスタイル
		CW_USEDEFAULT,          //表示X座標(OSに任せる)
		CW_USEDEFAULT,          //表示Y座標(OSに任せる)
		windowWidth,            //ウィンドウ横幅
		windowHeight,           //ウィンドウ縦幅
		nullptr,                //親ウィンドウハンドル
		nullptr,                //メニューハンドル
		wndClass.hInstance,     //呼び出しアプリケーションハンドル
		nullptr                 //オプション
	);

	// ウィンドウ表示
	return ShowWindow(hWnd, SW_SHOW);
}

int Win32API::WindowMessage()
{
	MSG msg{}; //メッセージ

	MessageCount++;

	// メッセージある?
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg); //キー入力メッセージの処理
		DispatchMessage(&msg); //プロシージャにメッセージを送る
	}

	// ×ボタンで終了メッセージが来たらゲームループを抜ける
	if (msg.message == WM_QUIT)
	{
		return -1;
	}

	return 0;
}

BOOL WINAPI Win32API::WindowEnd()
{
	//ウィンドウクラスを登録解除
	return UnregisterClass(wndClass.lpszClassName, wndClass.hInstance);
}