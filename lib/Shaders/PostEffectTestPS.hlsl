#include "PostEffectTest.hlsli"

Texture2D<float4> tex0 : register(t0); //0番スロットに設定されたテクスチャ
Texture2D<float4> tex1 : register(t1); //1番スロットに設定されたテクスチャ
SamplerState smp : register(s0);      //0番スロットに設定されたサンプラー

float4 main(VSOutput input) : SV_TARGET
{
	// UV指定したピクセルの色をサンプリング
	float4 texColor0 = tex0.Sample(smp, input.uv);
	float4 texColor1 = tex1.Sample(smp, input.uv);

	// ボーダーブレンド
	float4 color = texColor0;
	if (fmod(input.uv.y, 0.1f) < 0.05f)
	{
		color = texColor1;
	}

	// そのまま
	return float4(color.rgb, 1);

	// 明度の変更
	// float value = 2.0f;
	// return float4(texColor.rgb * value, 1);

	// ネガポジ反転
	// return float4(1 - texColor.rgb, 1);

	// ぼかし処理
	// const uint blurSize = 5;
	// const float uSize = 1.0f / 1280.0f;
	// const float vSize = 1.0f / 720.0f;
	// float4 color = {0, 0, 0, 0};
	// for (uint y = 0; y < blurSize; y++)
	// {
	// 	for (uint x = 0; x < blurSize; x++)
	// 	{
	// 		color += tex0.Sample(smp, input.uv + float2(x * uSize, y * vSize));
	// 	}
	// }
	// return float4(color.rgb / (blurSize * blurSize), 1);

	// テスト用
	//return float4(1, 1, 1, 1);
}