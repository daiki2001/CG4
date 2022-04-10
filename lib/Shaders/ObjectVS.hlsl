#include "ObjectData.hlsli"

VSOutput main(float4 pos : POSITION, float2 uv : TEXCOORD, float3 normal : NORMAL)
{
	VSOutput output; //ピクセルシェーダに渡す値
	output.svpos = mul(mat, pos); //座標に行列を乗算
	output.normal = normal;
	output.uv = uv;
	return output;
}