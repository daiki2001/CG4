#pragma once
#include <Windows.h>

class Win32API
{
public:
	// デフォルトコンストラクタ
	Win32API();
	// デストラクタ
	~Win32API() {}

public: // 静的メンバ関数
	// ウィンドウプロシージャ
	static LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// ウィンドウクラスの取得
	static WNDCLASSEX GetWindow() { return wndClass; }
	// ウィンドウハンドルの取得
	static HWND GetHwnd() { return hWnd; }

public: // 静的メンバ変数
#ifdef UNICODE
	static LPCWSTR className; //クラス名
	static LPCWSTR title; //タイトルバーの文字
#else
	static LPCSTR className; //クラス名
	static LPCSTR title; //タイトルバーの文字
#endif // UNICODE
private:
	static WNDCLASSEX wndClass;
	static HWND hWnd;

public: // メンバ関数
	// 初期化
	BOOL WINAPI WindowInit();
	// ウィンドウメッセージ
	int WindowMessage();
	// 終了処理
	BOOL WINAPI WindowEnd();

public: // メンバ変数
int windowWidth, windowHeight; //ウィンドウサイズ

bool windowInit; //初期化フラグ
UINT MessageCount; //WindowMessage()を通った回数

};
