#pragma once
#include "Vector3.h"

// •âŠÔŠÖ”
namespace Engine
{
namespace Math
{
// üŒ`•âŠÔ(1ŸŠÖ”•âŠÔ)
const Vector3 Lerp(const Vector3& start, const Vector3& end, const float t);

// easeIn•âŠÔ
const Vector3 easeIn(const Vector3& start, const Vector3& end, const float t);
// easeOut•âŠÔ
const Vector3 easeOut(const Vector3& start, const Vector3& end, const float t);
// easeInOut•âŠÔ
const Vector3 easeInOut(const Vector3& start, const Vector3& end, const float t);
} //Math
} //Engine
