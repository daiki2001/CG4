#pragma once
#include "BaseCollider.h"
#include "CollisionPrimitive.h"
#include "./Math/EngineMath.h"

class SphereCollider : public BaseCollider
{
private: // エイリアス
	using Vector3 = Engine::Math::Vector3;

public: // メンバ関数
	SphereCollider(const Vector3& offset = {0.0f, 0.0f, 0.0f}, const float& radius = 1.0f);

	// 更新
	void Update() override;

	// 半径の設定
	inline void SetRadius(const float& radius) { this->radius = radius; }

public: // メンバ変数
	Sphere sphere;
private:
	Vector3 offset; //オブジェクト中心からのオフセット
	float radius; //半径
};
