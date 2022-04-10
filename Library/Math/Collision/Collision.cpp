#include "Collision.h"

namespace
{
void ClosestPtPoint2Triangle(const EngineMath::Vector3& point, const Triangle& triangle, EngineMath::Vector3* closest)
{
	using namespace EngineMath;

	// pointがp0の外側の頂点領域の中にあるかどうかチェック
	Vector3 p0_p1 = triangle.p1 - triangle.p0;
	Vector3 p0_p2 = triangle.p2 - triangle.p0;
	Vector3 p0_pt = point - triangle.p0;

	float d1 = p0_p1.Dot(p0_pt);
	float d2 = p0_p2.Dot(p0_pt);

	if (d1 <= 0.0f && d2 <= 0.0f)
	{
		*closest = triangle.p0;
		return;
	}

	// pointがp1の外側の頂点領域の中にあるかどうかチェック
	Vector3 p1_pt = point - triangle.p1;

	float d3 = p0_p1.Dot(p1_pt);
	float d4 = p0_p2.Dot(p1_pt);

	if (d3 >= 0.0f && d4 <= d3)
	{
		*closest = triangle.p1;
		return;
	}

	// pointがp0_p1の辺領域の中にあるかどうかチェックし、あればpointのp0_p1上に対する射影を返す
	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
	{
		float v = d1 / (d1 - d3);
		*closest = triangle.p0 + v * p0_p1;
		return;
	}

	// pointがp2の外側の頂点領域の中にあるかどうかチェック
	Vector3 p2_pt = point - triangle.p2;

	float d5 = p0_p1.Dot(p2_pt);
	float d6 = p0_p2.Dot(p2_pt);

	if (d6 >= 0.0f && d5 <= d6)
	{
		*closest = triangle.p2;
		return;
	}

	// pointがp0_p2の辺領域の中にあるかどうかチェックし、あればpointのp0_p2上に対する射影を返す
	float vb = d5 * d2 - d1 * d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
	{
		float w = d2 / (d2 - d6);
		*closest = triangle.p0 + w * p0_p2;
		return;
	}

	// pointがp1_p2の辺領域の中にあるかどうかチェックし、あればpointのp1_p2上に対する射影を返す
	float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		*closest = triangle.p1 + w * (triangle.p2 - triangle.p1);
		return;
	}

	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	*closest = triangle.p0 + p0_p1 * v + p0_p2 * w;
}
}

namespace Collision
{
bool IsSphereToSphereCollision(const Sphere& sphere1, const Sphere& sphere2, Vector3* inter)
{
	static Vector3 distance{}; //中心点間の距離
	static bool result = false;

	distance = sphere2.center - sphere1.center;

	result = distance.Length() < (sphere1.radius + sphere2.radius);

	if (result)
	{
		if (inter != nullptr)
		{
			// Aの半径が0の時座標はBの中心 Bの半径が0の時座標はAの中心 となるよう補完
			float t = sphere2.radius / (sphere1.radius + sphere2.radius);
			*inter = EngineMath::Lerp(sphere1.center, sphere2.center, t);
		}
	}

	return result;
}

bool IsSphereToPlaneCollision(const Sphere& sphere, const Plane& plane)
{
	static float distance{}; //球体の中心座標と平面の距離
	distance = sphere.center.Dot(plane.normal) - plane.distance;
	if (fabsf(distance) > sphere.radius)
	{
		return false;
	}

	return true;
}

bool IsSphereToTriangleCollision(const Sphere& sphere, const Triangle& triangle)
{
	Vector3 p;
	ClosestPtPoint2Triangle(sphere.center, triangle, &p);
	Vector3 v = p - sphere.center;

	if (v.LengthSquared() > sphere.radius * sphere.radius)
	{
		return false;
	}

	return true;
}

bool IsRayToSphereCollision(const Ray& ray, const Sphere& sphere, float* distance, Vector3* inter)
{
	Vector3 m = ray.start - sphere.center;
	float b = m.Dot(ray.dir);
	float c = m.Dot(m) - sphere.radius * sphere.radius;

	// レイの始点が球体に当たって無くて、レイが球体に対して離れていく方向を指している場合は当たらない
	if ((c > 0.0f) && (b > 0.0f))
	{
		return false;
	}

	float discr = b * b - c;
	// 判別式がマイナスなら、レイは球体をそれている
	if (discr < 0.0f)
	{
		return false;
	}

	// 交差する場所tを求める
	float t = -b - sqrtf(discr);
	// tがマイナスの場合、始点が球体の内側にある
	if (t < 0)
	{
		t = 0.0f;
	}

	// 距離を書き込む
	if (distance != nullptr)
	{
		*distance = t;
	}
	// 交点を求める
	if (inter != nullptr)
	{
		*inter = ray.start + t * ray.dir;
	}

	return true;
}

bool IsRayToPlaneCollision(const Ray& ray, const Plane& plane, float* distance, Vector3* inter)
{
	// 面法線とレイの方向ベクトルの内積
	float d1 = plane.normal.Dot(ray.dir);
	// 面法線とレイの始点座標の内積
	float d2 = plane.normal.Dot(ray.start);
	// 始点と平面の距離(平面の法線方向)
	float dist = d2 - plane.distance;
	// 始点と平面の距離(レイ方向)
	float t = dist / -d1;
	// 交点が始点より後ろにあるから当たらない
	if (t < 0)
	{
		return false;
	}

	// 距離を書き込む
	if (distance != nullptr)
	{
		*distance = t;
	}
	// 交点を求める
	if (inter != nullptr)
	{
		*inter = ray.start + t * ray.dir;
	}

	return true;
}

bool IsRayToTriangleCollision(const Ray& ray, const Triangle& triangle, float* distance, Vector3* inter)
{
	// 三角形が乗っている平面を算出
	Plane plane;
	Vector3 interPlane;
	plane.normal = triangle.normal;
	plane.distance = triangle.normal.Dot(triangle.p0);
	// レイと平面が当たって無かったら当たって無い
	if (IsRayToPlaneCollision(ray, plane, distance, &interPlane) == false)
	{
		return false;
	}

	const float epsilon = 1.0e-5f; //誤差吸収用の微小な値
	Vector3 m;

	// 辺p0_p1
	Vector3 pt_p0 = triangle.p0 - interPlane;
	Vector3 p0_p1 = triangle.p1 - triangle.p0;
	m = pt_p0.Cross(p0_p1);

	if (m.Dot(triangle.normal) < -epsilon)
	{
		return false;
	}

	// 辺p1_p2
	Vector3 pt_p1 = triangle.p1 - interPlane;
	Vector3 p1_p2 = triangle.p2 - triangle.p1;
	m = pt_p1.Cross(p1_p2);

	if (m.Dot(triangle.normal) < -epsilon)
	{
		return false;
	}

	// 辺p2_p0
	Vector3 pt_p2 = triangle.p2 - interPlane;
	Vector3 p2_p0 = triangle.p0 - triangle.p2;
	m = pt_p2.Cross(p2_p0);

	if (m.Dot(triangle.normal) < -epsilon)
	{
		return false;
	}

	if (inter != nullptr)
	{
		*inter = interPlane;
	}

	return true;
}

bool IsOBBToOBBCollision(const Vector3& pos1, const XMMATRIX& rotation1, const Vector3& scale1, const Vector3& pos2, const XMMATRIX& rotation2, const Vector3& scale2)
{
	using namespace DirectX;

	// 二点間の距離
	static Vector3 distance{};
	// 中心から面までの長さ
	static Vector3 length1{}, length2{};
	// 分離軸ベクトル
	static Vector3 direction1_XN{}, direction1_YN{}, direction1_ZN{}, direction2_XN{}, direction2_YN{}, direction2_ZN{};
	// 方向ベクトル
	static Vector3 direction1_X{}, direction1_Y{}, direction1_Z{}, direction2_X{}, direction2_Y{}, direction2_Z{};

	/*OBBの当たり判定の下準備*/

	length1 = scale1 / 2.0f;
	XMStoreFloat3(&direction1_XN, XMVector4Normalize(rotation1.r[0]));
	XMStoreFloat3(&direction1_YN, XMVector4Normalize(rotation1.r[1]));
	XMStoreFloat3(&direction1_ZN, XMVector4Normalize(rotation1.r[2]));
	direction1_X = direction1_XN * length1.x;
	direction1_Y = direction1_YN * length1.y;
	direction1_Z = direction1_ZN * length1.z;

	length2 = scale2 / 2.0f;
	XMStoreFloat3(&direction2_XN, XMVector4Normalize(rotation2.r[0]));
	XMStoreFloat3(&direction2_YN, XMVector4Normalize(rotation2.r[1]));
	XMStoreFloat3(&direction2_ZN, XMVector4Normalize(rotation2.r[2]));
	direction2_X = direction2_XN * length2.x;
	direction2_Y = direction2_YN * length2.y;
	direction2_Z = direction2_ZN * length2.z;

	/*OBBの当たり判定*/

	distance = pos2 - pos1;

	static float r1{}, r2{}; //半投影線分
	static float l{}; //中間点間の距離

	// 分離軸ベクトル：direction1_XN
	r1 = direction1_X.Length();
	r2 = fabsf(direction1_XN.Dot(direction2_X)) + fabsf(direction1_XN.Dot(direction2_Y)) + fabsf(direction1_XN.Dot(direction2_Z));
	l = fabsf(direction1_XN.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：direction1_YN
	r1 = direction1_Y.Length();
	r2 = fabsf(direction1_YN.Dot(direction2_X)) + fabsf(direction1_YN.Dot(direction2_Y)) + fabsf(direction1_YN.Dot(direction2_Z));
	l = fabsf(direction1_YN.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：direction1_ZN
	r1 = direction1_Z.Length();
	r2 = fabsf(direction1_ZN.Dot(direction2_X)) + fabsf(direction1_ZN.Dot(direction2_Y)) + fabsf(direction1_ZN.Dot(direction2_Z));
	l = fabsf(direction1_ZN.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：direction2_XN
	r1 = direction2_X.Length();
	r2 = fabsf(direction2_XN.Dot(direction1_X)) + fabsf(direction2_XN.Dot(direction1_Y)) + fabsf(direction2_XN.Dot(direction1_Z));
	l = fabsf(direction2_XN.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：direction2_YN
	r1 = direction2_Y.Length();
	r2 = fabsf(direction2_YN.Dot(direction1_X)) + fabsf(direction2_YN.Dot(direction1_Y)) + fabsf(direction2_YN.Dot(direction1_Z));
	l = fabsf(direction2_YN.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：direction2_ZN
	r1 = direction2_Z.Length();
	r2 = fabsf(direction2_ZN.Dot(direction1_X)) + fabsf(direction2_ZN.Dot(direction1_Y)) + fabsf(direction2_ZN.Dot(direction1_Z));
	l = fabsf(direction2_ZN.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	static Vector3 crossDirection{}; //外積分離軸

	// 分離軸ベクトル：Cross(direction1_X, direction2_X)
	crossDirection = direction1_X.Cross(direction2_X);
	r1 = fabsf(crossDirection.Dot(direction1_Y)) + fabsf(crossDirection.Dot(direction1_Z));
	r2 = fabsf(crossDirection.Dot(direction2_Y)) + fabsf(crossDirection.Dot(direction2_Z));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：Cross(direction1_X, direction2_Y)
	crossDirection = direction1_X.Cross(direction2_Y);
	r1 = fabsf(crossDirection.Dot(direction1_Y)) + fabsf(crossDirection.Dot(direction1_Z));
	r2 = fabsf(crossDirection.Dot(direction2_X)) + fabsf(crossDirection.Dot(direction2_Z));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：Cross(direction1_X, direction2_Z)
	crossDirection = direction1_X.Cross(direction2_Z);
	r1 = fabsf(crossDirection.Dot(direction1_Y)) + fabsf(crossDirection.Dot(direction1_Z));
	r2 = fabsf(crossDirection.Dot(direction2_X)) + fabsf(crossDirection.Dot(direction2_Y));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：Cross(direction1_Y, direction2_X)
	crossDirection = direction1_Y.Cross(direction2_X);
	r1 = fabsf(crossDirection.Dot(direction1_X)) + fabsf(crossDirection.Dot(direction1_Z));
	r2 = fabsf(crossDirection.Dot(direction2_Y)) + fabsf(crossDirection.Dot(direction2_Z));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：Cross(direction1_Y, direction2_Y)
	crossDirection = direction1_Y.Cross(direction2_Y);
	r1 = fabsf(crossDirection.Dot(direction1_X)) + fabsf(crossDirection.Dot(direction1_Z));
	r2 = fabsf(crossDirection.Dot(direction2_X)) + fabsf(crossDirection.Dot(direction2_Z));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：Cross(direction1_Y, direction2_Z)
	crossDirection = direction1_Y.Cross(direction2_Z);
	r1 = fabsf(crossDirection.Dot(direction1_X)) + fabsf(crossDirection.Dot(direction1_Z));
	r2 = fabsf(crossDirection.Dot(direction2_X)) + fabsf(crossDirection.Dot(direction2_Y));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：Cross(direction1_Z, direction2_X)
	crossDirection = direction1_Z.Cross(direction2_X);
	r1 = fabsf(crossDirection.Dot(direction1_X)) + fabsf(crossDirection.Dot(direction1_Y));
	r2 = fabsf(crossDirection.Dot(direction2_Y)) + fabsf(crossDirection.Dot(direction2_Z));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：Cross(direction1_Z, direction2_Y)
	crossDirection = direction1_Z.Cross(direction2_Y);
	r1 = fabsf(crossDirection.Dot(direction1_X)) + fabsf(crossDirection.Dot(direction1_Y));
	r2 = fabsf(crossDirection.Dot(direction2_X)) + fabsf(crossDirection.Dot(direction2_Z));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	// 分離軸ベクトル：Cross(direction1_Z, direction2_Z)
	crossDirection = direction1_Z.Cross(direction2_Z);
	r1 = fabsf(crossDirection.Dot(direction1_X)) + fabsf(crossDirection.Dot(direction1_Y));
	r2 = fabsf(crossDirection.Dot(direction2_X)) + fabsf(crossDirection.Dot(direction2_Y));
	l = fabsf(crossDirection.Dot(distance));
	if ((r1 + r2) < l)
	{
		return false;
	}

	return true;
}

float sphereSwept(const Vector3& pos1, const Vector3& speed1, const float& r1, const Vector3& pos2, const Vector3& speed2, const float& r2)
{
	static const float noHit = -1.0f;

	static Vector3 endPos1{};       //t = 1.0f地点のpos1の中心位置
	static Vector3 endPos2{};       //t = 1.0f地点のpos2の中心位置
	static Vector3 startDistance{}; //t = 0.0f地点の中心点間の距離
	static Vector3 endDistance{};   //t = 1.0f地点の中心点間の距離
	static Vector3 distance{};      //二軸間の距離

	// t = 0.0f地点の中心点間の距離の計算
	startDistance = pos2 - pos1;

	// t = 0.0f地点の衝突判定
	if (distance.Length() < (r1 + r2))
	{
		return 0.0f;
	}

	// t = 1.0f地点のpos1の中心位置の計算
	endPos1 = pos1 + speed1;
	// t = 1.0f地点のpos2の中心位置の計算
	endPos2 = pos2 + speed2;
	// t = 1.0f地点の中心点間の距離の計算
	endDistance = endPos2 - endPos1;
	// t = 1.0f地点の中心点間の距離 - t = 0.0f地点の中心点間の距離
	distance = endDistance - startDistance;

	static float t = 0.0f; //衝突した時間
	static float p = 0.0f; //distanceの長さの二乗
	static float q = 0.0f; //startDistanceとdistanceの内積
	static float r = 0.0f; //startDistanceの長さの二乗

	// distanceの長さの二乗の計算
	p = distance.LengthSquared();
	if (p == 0)
	{
		// distanceの長さが0なら衝突していない
		return noHit;
	}

	// startDistanceの長さの二乗の計算
	r = startDistance.LengthSquared();
	// startDistanceとdistanceの内積の計算
	q = startDistance.Dot(distance);

	float keep = (q * q) - p * (r - ((r1 + r2) * (r1 + r2))); //ルート内の計算
	if (keep < 0)
	{
		// ルート内がマイナスの場合は衝突しない
		return noHit;
	}
	else
	{
		// 衝突時間の計算
		t = (-q - sqrtf(keep)) / p;
		// ルート内がプラスで求めたtが0~1の間に無い場合は式を変えて計算し直す
		if (keep > 0 && (t < 0.0f || t > 1.0f))
		{
			t = (-q + sqrtf(keep)) / p;
		}
	}

	if (t >= 0.0f && t <= 1.0f)
	{
		return t;
	}
	else
	{
		return noHit;
	}
}

bool IsPredictCollisionBall(const Vector3& pos1, const Vector3& speed1, const float& r1, const Vector3& pos2, const Vector3& speed2, const float& r2)
{
	// 二点間の距離
	static Vector3 distance{};
	// 中心点間の距離
	distance = pos1 - pos2;

	return distance.Length() < ((r1 + speed1.Length()) + (r2 + speed2.Length()));
}
} // namespace Collision
