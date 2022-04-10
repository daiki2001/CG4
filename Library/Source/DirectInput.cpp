#include "./Header/DirectInput.h"
#include "./Header/Win32API.h"
#include <cassert>

#define EoF (-1) // End of Function

const LONG DirectInput::zoneMax = +1000;
const LONG DirectInput::zoneMin = -1000;

IDirectInput8* DirectInput::dinput = nullptr;
IDirectInputDevice8* DirectInput::devkeyborad = nullptr;
std::vector<IDirectInputDevice8*> DirectInput::devgamepad = {};
BYTE DirectInput::key[256] = {};
std::vector<DIJOYSTATE> DirectInput::gamepad = {};
std::vector<DIDEVICEINSTANCE> DirectInput::parameter = {};

DirectInput* DirectInput::GetInstance()
{
	// インスタンスの生成
	static DirectInput instance;

	return &instance;
}

BOOL CALLBACK DirectInput::EnumGamepadCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	std::vector<DIDEVICEINSTANCE>* ptr = (std::vector<DIDEVICEINSTANCE>*)pvRef;
	DIDEVICEINSTANCE tmp = *lpddi;

	// 配列に格納
	ptr->push_back(tmp);

	return DIENUM_CONTINUE;
}

int DirectInput::Init()
{
	HRESULT hr;

	// DirectInputインターフェースの生成
	hr = DirectInput8Create(Win32API::GetWindow().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("DirectInputインターフェースの作成に失敗しました\n");
#endif // _DEBUG

		return EoF;
	}

	// キーボードデバイスの生成
	hr = dinput->CreateDevice(GUID_SysKeyboard, &devkeyborad, NULL);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("キーボードデバイスの作成に失敗しました\n");
#endif // _DEBUG

		return EoF;
	}

	// 入力データ形式のセット
	hr = devkeyborad->SetDataFormat(&c_dfDIKeyboard); //標準形式
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("キーボードデバイスのデータ形式の設定に失敗しました\n");
#endif // _DEBUG

		return EoF;
	}

	// 協調レベルのセット
	hr = devkeyborad->SetCooperativeLevel(Win32API::GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("キーボードデバイスの協調レベルの設定に失敗しました\n");
#endif // _DEBUG

		return EoF;
	}

	// ゲームパッドデバイスの生成
	hr = CreateGamePadDevice();
	if (FAILED(hr))
	{
		return EoF;
	}

	return 0;
}

int DirectInput::Update()
{
	if (KeyInputUpdate() == EoF)
	{
		return EoF;
	}
	if (GamePadInputUpdate() == EoF)
	{
		return EoF;
	}

	return 0;
}

void DirectInput::Release()
{
	// キーボードデバイスの制御を停止させる
	devkeyborad->Unacquire();
	// キーボードデバイスを解放する
	devkeyborad->Release();

	for (size_t i = 0; i < devgamepad.size(); i++)
	{
		// 各ゲームパッドデバイスの制御を停止させる
		devgamepad[i]->Unacquire();
		// 各ゲームパッドデバイスを解放する
		devgamepad[i]->Release();
	}

	// DirectInputを解放する
	dinput->Release();
}

bool DirectInput::CheckHitKey(int keyCode)
{
	if (keyCode < 0 || keyCode >= sizeof(key))
	{
		return false;
	}

	if (KeyInputUpdate() != 0)
	{
		return false;
	}

	if (key[keyCode])
	{
		return true;
	}

	return false;
}

int DirectInput::CheckHitKeyAll(int CheckType)
{
	if ((CheckType & CHECKINPUT_KEY) != 0)
	{
		for (int i = 0; i < sizeof(key) / sizeof(key[0]); i++)
		{
			if (CheckHitKey(i) == true)
			{
				return 0;
			}
		}
	}
	if ((CheckType & CHECKINPUT_PAD) != 0)
	{

	}
	if ((CheckType & CHECKINPUT_MOUSE) != 0)
	{

	}

	return -1;
}

int DirectInput::GetHitKeyStateAll(char* KeyStateArray)
{
	if (KeyStateArray == nullptr)
	{
		return -1;
	}

	for (int i = 0; i < sizeof(key) / sizeof(key[0]); i++)
	{
		if (CheckHitKey(i) == true)
		{
			KeyStateArray[i] = (char)true;
		}
		else
		{
			KeyStateArray[i] = (char)false;
		}
	}

	return 0;
}

int DirectInput::GetGamepadStateAll(DIJOYSTATE* gamepadState, const size_t& gamepadIndex)
{
	if (gamepadIndex >= gamepad.size())
	{
		return EoF;
	}

	*gamepadState = gamepad[gamepadIndex];

	return 0;
}

int DirectInput::KeyInputUpdate()
{
	HRESULT hr;

	ZeroMemory(key, sizeof(key));
	hr = devkeyborad->GetDeviceState(sizeof(key), key);
	if (FAILED(hr)) {
		// 失敗なら再開させてもう一度取得
		devkeyborad->Acquire();
		devkeyborad->GetDeviceState(sizeof(key), key);
	}

	return 0;
}

int DirectInput::GamePadInputUpdate(const size_t& gamepadNo)
{
	HRESULT hr;

	if (gamepadNo > devgamepad.size())
	{
#ifdef _DEBUG
		OutputDebugStringA("指定されたゲームパッドはありません\n");
#endif // _DEBUG

		assert(0);
		return EoF;
	}

	if (gamepadNo == devgamepad.size())
	{
		hr = CreateGamePadDevice();
		if (FAILED(hr))
		{
			return EoF;
		}
	}

	hr = devgamepad[gamepadNo]->Acquire();
	if (hr == DIERR_INPUTLOST)
	{
		// デバイスがロストしていたら関数を終了する
		return EoF;
	}

	devgamepad[gamepadNo]->Poll();

	for (size_t i = gamepad.size(); i < gamepadNo; i++)
	{
		gamepad.push_back({});
	}
	ZeroMemory(&gamepad[gamepadNo], sizeof(DIJOYSTATE));
	devgamepad[gamepadNo]->GetDeviceState(sizeof(DIJOYSTATE), &gamepad[gamepadNo]);

	return 0;
}

HRESULT DirectInput::CreateGamePadDevice()
{
	HRESULT hr = S_OK;

	// ゲームパッドデバイスの列挙
	hr = dinput->EnumDevices(DI8DEVTYPE_GAMEPAD, &EnumGamepadCallback, &parameter, DIEDFL_ATTACHEDONLY);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("ゲームパッドデバイスの列挙に失敗しました\n");
#endif // _DEBUG

		return hr;
	}

	for (size_t i = 0; i < parameter.size(); i++)
	{
		devgamepad.push_back({});

		// ゲームパッドデバイスの生成
		hr = dinput->CreateDevice(parameter[i].guidInstance, &devgamepad[i], NULL);
		if (FAILED(hr))
		{
#ifdef _DEBUG
			OutputDebugStringA("ゲームパッドデバイスの生成に失敗しました\n");
#endif // _DEBUG

			return hr;
		}

		// 入力データ形式のセット
		hr = devgamepad[i]->SetDataFormat(&c_dfDIJoystick);
		if (FAILED(hr))
		{
#ifdef _DEBUG
			OutputDebugStringA("ゲームパッドデバイスのデータ形式の設定に失敗しました\n");
#endif // _DEBUG

			return hr;
		}

		// プロパティの設定
		hr = SetUpGamePadProperty(i);
		if (FAILED(hr))
		{
#ifdef _DEBUG
			OutputDebugStringA("ゲームパッドデバイスのプロパティの設定に失敗しました\n");
#endif // _DEBUG

			return hr;
		}

		// 協調レベルの設定
		hr = devgamepad[i]->SetCooperativeLevel(Win32API::GetHwnd(), DISCL_FOREGROUND | DISCL_EXCLUSIVE);
		if (FAILED(hr))
		{
#ifdef _DEBUG
			OutputDebugStringA("ゲームパッドデバイスの協調レベルの設定に失敗しました\n");
#endif // _DEBUG

			return hr;
		}
	}

	return hr;
}

HRESULT DirectInput::SetUpGamePadProperty(const size_t& gamepadNo)
{
	if (gamepadNo >= devgamepad.size())
	{
#ifdef _DEBUG
		OutputDebugStringA("指定されたゲームパッドはありません\n");
#endif // _DEBUG

		return S_FALSE;
	}

	HRESULT hr;

	// 軸モードを絶対値モードとして設定
	DIPROPDWORD diprop;
	ZeroMemory(&diprop, sizeof(diprop));
	diprop.diph.dwSize = sizeof(diprop);
	diprop.diph.dwHeaderSize = sizeof(diprop.diph);
	diprop.diph.dwHow = DIPH_DEVICE;
	diprop.diph.dwObj = 0;
	diprop.dwData = DIPROPAXISMODE_ABS;
	hr = devgamepad[gamepadNo]->SetProperty(DIPROP_AXISMODE, &diprop.diph);
	if (FAILED(hr))
	{
		return hr;
	}

	// X軸の値の範囲設定
	DIPROPRANGE diprg;
	ZeroMemory(&diprg, sizeof(diprg));
	diprg.diph.dwSize = sizeof(diprg);
	diprg.diph.dwHeaderSize = sizeof(diprg.diph);
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.diph.dwObj = DIJOFS_X;
	diprg.lMin = zoneMin;
	diprg.lMax = zoneMax;
	hr = devgamepad[gamepadNo]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("X軸の設定に失敗しました\n");
#endif // _DEBUG
	}

	// Y軸の値の範囲設定
	diprg.diph.dwObj = DIJOFS_Y;
	hr = devgamepad[gamepadNo]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("Y軸の設定に失敗しました\n");
#endif // _DEBUG
	}

	// Z軸の値の範囲設定
	diprg.diph.dwObj = DIJOFS_Z;
	hr = devgamepad[gamepadNo]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("Z軸の設定に失敗しました\n");
#endif // _DEBUG
	}

	// RX軸の値の範囲設定
	diprg.diph.dwObj = DIJOFS_RX;
	hr = devgamepad[gamepadNo]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("RX軸の設定に失敗しました\n");
#endif // _DEBUG
	}

	// RY軸の値の範囲設定
	diprg.diph.dwObj = DIJOFS_RY;
	hr = devgamepad[gamepadNo]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("RY軸の設定に失敗しました\n");
#endif // _DEBUG
	}

	// RZ軸の値の範囲設定
	diprg.diph.dwObj = DIJOFS_RZ;
	hr = devgamepad[gamepadNo]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugStringA("RZ軸の設定に失敗しました\n");
#endif // _DEBUG
	}

	return S_OK;
}