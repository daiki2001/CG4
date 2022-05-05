#include "./Header/Model.h"

Node::Node() :
	name{},
	position{ 0.0f, 0.0f, 0.0f },
	rotation{ EngineMath::Identity() },
	scaling{ 1.0f, 1.0f, 1.0f },
	transform{ EngineMath::Identity() },
	globalTransform{ EngineMath::Identity() },
	parent{ nullptr }
{
}
