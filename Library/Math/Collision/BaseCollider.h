#pragma once
#include "CollisionTypes.h"
#include "./Header/DirectDrawing.h"
#include "./Math/Collision/CollisionInfo.h"

class BaseCollider
{
private: // エイリアス
	using ShapeType = Collision::ShapeType;

public: // メンバ関数
	BaseCollider() {}
	virtual ~BaseCollider() {}

	// 更新
	virtual void Update() = 0;
	// 形状タイプの取得
	inline ShapeType GetShapeType() const { return shapeType; }

	// オブジェクトのデータの設定
	inline void SetObject(Object* objectData) { this->objectData = objectData; }
	// オブジェクトのデータの取得
	inline Object* GetObject() const { return objectData; }
	// 衝突時コールバック関数
	inline void OnCollision(const CollisionInfo& info) { objectData->OnCollision(info); }

protected: // メンバ変数
	Object* objectData = nullptr;                            //オブジェクトのデータ
	ShapeType shapeType = ShapeType::SHAPE_UNKNOWN; //形状タイプ
};
