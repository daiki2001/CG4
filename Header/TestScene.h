#pragma once
#include "./Header/BaseScene.h"
#include "./Math/EngineMath.h"
#include "./Math/Collision/Collision.h"
#include "Player.h"
#include "./Header/FbxLoader.h"

class TestScene : public BaseScene
{
public: // エイリアス
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	using Vector3 = Engine::Math::Vector3;

public: // 定数
	static const float sphereRadius; //球体オブジェクトの半径
	static const float planeRadius;  //平面オブジェクトの大きさ
	static const Vector3 tPos1;      //三角形オブジェクトの頂点1
	static const Vector3 tPos2;      //三角形オブジェクトの頂点2
	static const Vector3 tPos3;      //三角形オブジェクトの頂点3

	static const XMFLOAT4 color[3];

	static const float rayLength; //レイの長さ

	static const float triangleAngle;
	static const float triangleScale;

public: // メンバ関数
	TestScene(SceneChenger* sceneChenger);
	~TestScene() override;

	void Init() override;
	void Update() override;
	void Draw() override;

private: // メンバ変数
	FbxLoader* fbxLoader;

	int background; //背景画像

	// オブジェクトの生成
	int sphere;   //球体オブジェクト
	int plane;    //平面オブジェクト
	int triangle; //三角形オブジェクト
	int ray;      //レイ描画用オブジェクト

	Player* player;
	Vector3 playerPos;

	Vector3 rayPos;
	Vector3 rayDir;
	Vector3 rayAngle;
	XMMATRIX rayRot;
	float rayDistance;

	Vector3 spherePos;

	Vector3 planePos;
	XMMATRIX planeRot;
	float planeScale;
	Vector3 planeToRayHitPos;

	Vector3 trianglePos;
	XMMATRIX triangleRot;
	XMMATRIX triangleInvRot;
	XMMATRIX triangleMatrix;

	Sphere playerCollision;
	Sphere sphereCollision;
	Plane planeCollision;
	Triangle triangleCollision;
	Ray rayCollision;

	bool isCameraMove;
	bool isRay;
};
