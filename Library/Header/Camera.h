#pragma once
#include "./Math/EngineMath.h"

class Camera
{
private: // エイリアス
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;
	using Vector3 = Engine::Math::Vector3;

public: // 静的メンバ変数
	static float targetRadius; //注視点からの距離
	static float longitude;    //経度
	static float latitude;     //緯度

	static Vector3 pos;    //カメラの位置
	static Vector3 target; //カメラの注視点
	static Vector3 upVec;        //上ベクトル

	/*メンバ関数*/
public:
	Camera() {}
	~Camera() {}

	// キャラクタ姿勢行列の生成
	static DirectX::XMMATRIX CreateCamera(const XMVECTOR& pos, const XMVECTOR& target, const XMVECTOR& upVector);
	// キャラクタ姿勢束縛行列の生成
	static DirectX::XMMATRIX CreateCameraFix(const XMVECTOR& pos, const XMVECTOR& target, const XMVECTOR& upVector);
};
