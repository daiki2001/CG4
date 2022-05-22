#pragma once
#include "./Math/EngineMath.h"

struct BaseCollision
{
	bool isHit = false;

	BaseCollision() {}
};

// 球体
struct Sphere : BaseCollision
{
private: // エイリアス
	using Vector3 = Engine::Math::Vector3;

public:
	Vector3 center = Vector3(0.0f, 0.0f, 0.0f);
	float radius = 1.0f;

	Sphere() = default;
	Sphere(Vector3 pos, float r) :
		BaseCollision()
	{
		center = pos;
		radius = r;
	}
};

// 平面
struct Plane : BaseCollision
{
private: // エイリアス
	using Vector3 = Engine::Math::Vector3;

public:
	Vector3 normal = Vector3(0.0f, 0.0f, -1.0f);
	float distance = 0.0f;

	Plane() = default;
	Plane(Engine::Math::Matrix4 rotation, float distance) :
		BaseCollision()
	{
		this->distance = distance;

		static const Vector3 baseNormal = Vector3(0.0f, 0.0f, -1.0f);
		normal = baseNormal * rotation;
		normal = -normal.Normalize();
	}
};

// 法線付き三角形
class Triangle : public BaseCollision
{
private: // エイリアス
	using Vector3 = Engine::Math::Vector3;

public:
	Vector3 p0 = Vector3();
	Vector3 p1 = Vector3();
	Vector3 p2 = Vector3();
	Vector3 normal = Vector3(0.0f, 1.0f, 0.0f);

public:
	Triangle() = default;
	Triangle(Vector3 p0, Vector3 p1, Vector3 p2) :
		BaseCollision()
	{
		this->p0 = p0;
		this->p1 = p1;
		this->p2 = p2;

		ComputeNormal();
	}

	// 法線の計算
	void ComputeNormal();
};

// レイ(半直線)
struct Ray : BaseCollision
{
private: // エイリアス
	using Vector3 = Engine::Math::Vector3;

public:
	Vector3 start = Vector3();
	Vector3 dir = Vector3(1.0f, 0.0f, 0.0f);

	Ray() = default;
	Ray(Vector3 start, Vector3 dir) :
		BaseCollision()
	{
		this->start = start;
		this->dir = dir.Normalize();
	}
};