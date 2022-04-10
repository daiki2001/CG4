#include "ObjectData.hlsli"

Texture2D<float4> tex : register(t0); //0番スロットに設定されたテクスチャ
SamplerState smp : register(s0);      //0番スロットに設定されたサンプラー

float4 main(VSOutput input) : SV_TARGET
{
	float4 texColor = float4(tex.Sample(smp, input.uv));
	float3 light = normalize(lightVec); //光源へのベクトルを正規化
	float diffuse = saturate(dot(-light, input.normal)); //光源へのベクトルと法線ベクトルの内積([0, 1]の範囲にクランプ)
	float brightness = diffuse + 0.3f; //アンビエント項を0.3として計算
	return float4(texColor.rgb * brightness, texColor.a) * color;
	//return color;
}