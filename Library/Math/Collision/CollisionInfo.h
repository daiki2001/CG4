#pragma once
#include "./Math/EngineMath.h"

class BaseCollider;

struct CollisionInfo
{
private: // エイリアス
	using Vector3 = Engine::Math::Vector3;

public:
	CollisionInfo(BaseCollider* collider, const Vector3& inter)
	{
		this->collider = collider;
		this->inter = inter;
	}

	BaseCollider* collider = nullptr; //衝突相手のコライダー
	Vector3 inter;                    //衝突点
};