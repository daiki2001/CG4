#include "SphereCollider.h"

SphereCollider::SphereCollider(const Vector3& offset, const float& radius) :
	BaseCollider(),
	offset(offset),
	radius(radius)
{
	shapeType = Collision::ShapeType::SHAPE_SPHERE;
}

void SphereCollider::Update()
{
	const Vector3& pos = Vector3(
		objectData->matWorld.r[3].m128_f32[0],
		objectData->matWorld.r[3].m128_f32[1],
		objectData->matWorld.r[3].m128_f32[2]);

	sphere.center = pos + offset;
	sphere.radius = radius;
}
