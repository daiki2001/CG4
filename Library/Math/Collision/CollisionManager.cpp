#include "CollisionManager.h"
#include "BaseCollider.h"
#include "Collision.h"

CollisionManager* CollisionManager::Get()
{
	static CollisionManager instance;
	return &instance;
}

void CollisionManager::CheckAllCollisions()
{
	using namespace Collision;

	std::forward_list<BaseCollider*>::iterator it1;
	std::forward_list<BaseCollider*>::iterator it2;

	it1 = colliders.begin();
	for (; it1 != colliders.end(); ++it1)
	{
		it2 = it1;
		++it2;
		for (; it2 != colliders.end(); ++it2)
		{
			BaseCollider* col1 = *it1;
			BaseCollider* col2 = *it2;

			if (col1->GetShapeType() == ShapeType::SHAPE_SPHERE &&
				col2->GetShapeType() == ShapeType::SHAPE_SPHERE)
			{
				SphereSphere(col1, col2);
			}
		}
	}
}

void CollisionManager::SphereSphere(BaseCollider* col1, BaseCollider* col2)
{
	Sphere* sphereA = dynamic_cast<Sphere*>(col1);
	Sphere* sphereB = dynamic_cast<Sphere*>(col2);
	Engine::Math::Vector3 inter;

	if (Collision::IsSphereToSphereCollision(*sphereA, *sphereB, &inter))
	{
		col1->OnCollision(CollisionInfo(col2, inter));
		col2->OnCollision(CollisionInfo(col1, inter));
	}
}
