#pragma once
#include <DirectXMath.h>

// インクルードファイル
#include "./Math/Vector3.h"
#include "./Math/Matrix4.h"
#include "./Math/Easing.h"

/*定数*/
namespace Engine
{
namespace Math
{
static constexpr float degree = DirectX::XM_PI / 180.0f; //1°

static constexpr float gravity = 9.8f; //重力加速度
} //Math
} //Engine

/*オペレーター演算子オーバーロード*/
namespace Engine
{
namespace Math
{
bool operator==(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b);
bool operator!=(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b);
} //Math
} //Engine
