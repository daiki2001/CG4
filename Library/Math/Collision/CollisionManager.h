#pragma once
#include <forward_list>

class BaseCollider;

class CollisionManager final
{
public: // シングルトン化
	static CollisionManager* Get();
private:
	CollisionManager() {}
	CollisionManager(const CollisionManager&) = delete;
	~CollisionManager() {}
	CollisionManager operator=(const CollisionManager&) = delete;

public: // メンバ関数
	// コライダーの追加
	inline void AddCollider(BaseCollider* collider) { colliders.push_front(collider); }
	// コライダーの削除
	inline void RemoveCollider(BaseCollider* collider) { colliders.remove(collider); }

	// 全ての衝突チェック
	void CheckAllCollisions();
private:
	// 球体同士の衝突時処理
	void SphereSphere(BaseCollider* col1, BaseCollider* col2);

private: // メンバ変数
	std::forward_list<BaseCollider*> colliders; //コライダーのリスト
};
