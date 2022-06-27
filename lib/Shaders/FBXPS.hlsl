#include "FBX.hlsli"

struct PSOutput
{
	float4 target0 : SV_Target0;
	float4 target1 : SV_Target1;
};

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

PSOutput main(VSOutput input)
{
	PSOutput output;

	// テクスチャマッピング
	float4 texColor = tex.Sample(smp, input.uv);
	// Lambert反射
	float3 light = normalize(float3(1.0f, -1.0f, 1.0f));
	float  diffuse = saturate(dot(-light, input.normal));
	float  brightNess = diffuse + 0.3f;
	float4 shadeColor = float4(brightNess, brightNess, brightNess, 1.0f);

	output.target0 = shadeColor + texColor;
	output.target1 = float4(1 -(shadeColor + texColor).rgb, 1);
	return output;
}
