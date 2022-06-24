cbuffer cbuff0 : register(b0)
{
}

// 頂点シェーダーからの出力情報
struct VSOutput
{
	float4 svpos  : SV_POSITION; //頂点座標
	float2 uv     : TEXCOORD;    //uv座標
};