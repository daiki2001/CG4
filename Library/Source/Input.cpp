#include "./Header/Input.h"
#include "./Header/DirectInput.h"

DirectInput* Input::input = {};
char Input::keys[256] = {};
char Input::oldkeys[256] = {};

void Input::Init()
{
	input = DirectInput::GetInstance();
	DirectInput::Init();
}

void Input::Update()
{
	for (size_t i = 0; i < 256; i++)
	{
		oldkeys[i] = keys[i];
	}
	input->GetHitKeyStateAll(keys);
}

bool Input::IsKey(int keyCode)
{
	return keys[keyCode];
}

bool Input::IsKeyTrigger(int keyCode)
{
	return keys[keyCode] && !oldkeys[keyCode];
}

bool Input::IsKeyReturn(int keyCode)
{
	return !keys[keyCode] && oldkeys[keyCode];
}
