#pragma once
#include "DirectInput.h"
#include "Win32API.h"

class Input final
{
private:
	// コンストラクタをprivateにする
	Input() {}
	// デストラクタをprivateにする
	~Input() {}
public:
	// コピーコンストラクタの削除
	Input(const Input& obj) = delete;
	// 代入演算子の削除
	const Input& operator=(const Input& obj) = delete;

public: // 静的メンバ関数
	// 初期化
	static void Init();
	// 更新
	static void Update();
	// キーが押されているかどうか
	static bool IsKey(int key);
	// キーが押されたかどうか
	static bool IsKeyTrigger(int key);
	// キーから離れたかどうか
	static bool IsKeyReturn(int key);

private: // 静的メンバ変数
	// 入力デバイス
	static DirectInput* input;
	// 最新のキーボード情報用
	static char keys[256];
	// 1ループ(フレーム)前のキーボード情報
	static char oldkeys[256];

};
