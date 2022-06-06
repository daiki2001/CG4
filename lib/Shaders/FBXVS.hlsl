#include "FBX.hlsli"

VSOutput main(VSInput input)
{
	// 法線にワールド行列を適用
	float4 wnormal = normalize(mul(world, float4(input.normal, 0)));

	VSOutput output;
	output.svpos = mul(mul(viewproj, world), input.pos);
	output.normal = wnormal.xyz;
	output.uv = input.uv;
	return output;
}
