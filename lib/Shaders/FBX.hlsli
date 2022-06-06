cbuffer cbuff0 : register(b0)
{
	matrix viewproj;  //ビュープロジェクション行列
	matrix world;     //ワールド行列
	float3 cameraPos; //カメラ座標
}

struct VSInput
{
	float4 pos    : POSITION; //位置
	float3 normal : NORMAL;   //法線ベクトル
	float2 uv     : TEXCOORD; //テクスチャ座標
};

struct VSOutput
{
	float4 svpos  : SV_POSITION; //位置
	float3 normal : NORMAL;      //法線ベクトル
	float2 uv     : TEXCOORD;    //テクスチャ座標
};
