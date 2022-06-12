cbuffer cbuff0 : register(b0)
{
	matrix viewproj;  //ビュープロジェクション行列
	matrix world;     //ワールド行列
	float3 cameraPos; //カメラ座標
}

static const int MAX_BONES = 32; // ボーンの最大数

cbuffer skining : register(b3)
{
	matrix matSkinning[MAX_BONES]; //ボーンのスキニング行列
}

struct VSInput
{
	float4 pos         : POSITION;    //位置
	float3 normal      : NORMAL;      //法線ベクトル
	float2 uv          : TEXCOORD;    //テクスチャ座標
	uint4 boneIndices  : BONEINDICES; //ボーンの番号
	float4 boneWeights : BONEWEIGHTS; //ボーンのスキンウェイト
};

struct VSOutput
{
	float4 svpos  : SV_POSITION; //位置
	float3 normal : NORMAL;      //法線ベクトル
	float2 uv     : TEXCOORD;    //テクスチャ座標
};
