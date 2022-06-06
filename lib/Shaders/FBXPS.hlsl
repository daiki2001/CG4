#include "FBX.hlsli"

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

float4 main(VSOutput input) : SV_Target
{
	// テクスチャマッピング
	float4 texColor = tex.Sample(smp, input.uv);
	// Lambert反射
	float3 light = normalize(float3(1.0f, -1.0f, 1.0f));
	float  diffuse = saturate(dot(-light, input.normal));
	float  brightNess = diffuse + 0.3f;
	float4 shadeColor = float4(brightNess, brightNess, brightNess, 1.0f);

	return shadeColor + texColor;
	// return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
