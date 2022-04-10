#pragma once
#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include <dinput.h>
#include <vector>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// ゲームパッドの十字キー入力
#define PAD_POVLEFT   (28)
#define PAD_POVRIGHT  (29)
#define PAD_POVUP     (30)
#define PAD_POVDOWN   (31)

// CheckHitKeyAll で調べる入力タイプ
#define CHECKINPUT_KEY   (0x01) //キー入力を調べる
#define CHECKINPUT_PAD   (0x02) //パッド入力を調べる
#define CHECKINPUT_MOUSE (0x04) //マウスボタン入力を調べる
#define CHECKINPUT_ALL   (CHECKINPUT_KEY | CHECKINPUT_PAD | CHECKINPUT_MOUSE) //すべての入力を調べる

class DirectInput final
{
private:
	// コンストラクタをprivateにする
	DirectInput() {};
	// デストラクタをprivateにする
	~DirectInput() {}
public:
	// コピーコンストラクタを無効にする
	DirectInput(const DirectInput& obj) = delete;
	// 代入演算子を無効にする
	const DirectInput& operator=(const DirectInput& obj) = delete;
	// インスタンスの取得
	static DirectInput* GetInstance();

public: // 定数
	static const LONG zoneMax; //スティック入力の上限値
	static const LONG zoneMin; //スティック入力の下限値

public: // 静的メンバ関数
	// 初期化
	static int Init();
	// 更新
	static int Update();
	// デバイスの解放
	static void Release();

	// 特定のキーボードの入力状態の取得
	static bool CheckHitKey(int keyCode);
	// 全ての入力デバイスの状態の取得
	static int CheckHitKeyAll(int CheckType = CHECKINPUT_ALL);
	// キーボードデバイスの入力状態の取得
	static int GetHitKeyStateAll(char* KeyStateArray);
	// 特定のゲームパッドデバイスの生の入力状態の取得
	static int GetGamepadStateAll(DIJOYSTATE* gamepadState, const size_t& gamepadIndex = 0);
private:
	// デバイスを列挙する時のコールバック関数
	static BOOL CALLBACK EnumGamepadCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

	// キーボードデバイスの更新
	static int KeyInputUpdate();
	// ゲームパッドデバイスの更新
	static int GamePadInputUpdate(const size_t& gamepadNo = 0);
	// ゲームパッドデバイスの生成
	static HRESULT CreateGamePadDevice();
	// プロパティの設定
	static HRESULT SetUpGamePadProperty(const size_t& gamepadNo = 0);

private: // 静的メンバ変数
	// DirectInputインターフェース
	static IDirectInput8* dinput;
	// キーボードデバイス
	static IDirectInputDevice8* devkeyborad;
	// ゲームパッドデバイス
	static std::vector<IDirectInputDevice8*> devgamepad;

	// キーボードの入力情報
	static BYTE key[256];
	// ゲームパッドの入力情報
	static std::vector<DIJOYSTATE> gamepad;

	static std::vector<DIDEVICEINSTANCE> parameter;

};
