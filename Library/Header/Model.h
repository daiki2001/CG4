#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <fbxsdk.h>
#include <string>
#include <vector>
#include <DirectXTex.h>
#include "./Math/EngineMath.h"
#include "./Camera.h"

struct Node
{
	std::string name;                      //名前
	Engine::Math::Vector3 position;        //ローカル座標
	Engine::Math::Matrix4 rotation;        //ローカル回転行列
	Engine::Math::Vector3 scaling;         //ローカルスケール
	Engine::Math::Matrix4 transform;       //ローカル変換行列
	Engine::Math::Matrix4 globalTransform; //グローバル変換行列
	Node* parent;                          //親ノード

	Node();
};

class Model
{
private: // エイリアス
	using Vector3 = Engine::Math::Vector3;
	using Matrix4 = Engine::Math::Matrix4;
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	template<class T> using vector = std::vector<T>;

public: // フレンドクラス
	friend class FbxLoader;

public: // 定数
	static const int MAX_BONE_INDICES = 4;
	static const int MAX_BONE = 32;

public: // サブクラス
	// 頂点データ構造体
	struct VertexPosNormalUvSkin
	{
		Vector3 pos;          //xyz座標
		Vector3 normal;       //法線ベクトル
		DirectX::XMFLOAT2 uv; //uv座標

		UINT boneIndex[MAX_BONE_INDICES];   //ボーン 番号
		float boneWeight[MAX_BONE_INDICES]; //ボーン 重み
	};
	// 定数バッファ用データ構造体
	struct ConstBufferData
	{
		Matrix4 viewProj;  //ビュープロジェクション行列
		Matrix4 world;     //ワールド行列
		Vector3 cameraPos; //カメラ座標
	};
	// ボーン構造体
	struct Bone
	{
		std::string name = {};                          //名前
		Matrix4 invInitPose = Engine::Math::Identity(); //初期姿勢の逆行列
		FbxCluster* fbxCluster = nullptr;               //クラスター
	};
	// 定数バッファ用データ構造体(スキニング)
	struct ConstBufferDataSkin
	{
		Matrix4 bones[MAX_BONE]; //ボーンのスキニング行列
	};

private: // 静的メンバ変数
	static ID3D12Device* dev;

	static ComPtr<ID3D12RootSignature> rootSignature; //ルートシグネチャ
	static ComPtr<ID3D12PipelineState> pipelineState; //パイプラインステート

public: // 静的メンバ関数
	static HRESULT CreateGraphicsPipeline();

public: // メンバ変数
	Vector3 pos;   //ローカル座標
	Matrix4 rota;  //回転行列
	Vector3 scale; //スケール
	Matrix4 world; //ワールド行列

	FbxTime frameTime;   //1フレームの時間
	FbxTime startTime;   //アニメーション開始時間
	FbxTime endTime;     //アニメーション終了時間
	FbxTime currentTime; //現在時間
	bool isPlay;         //アニメーション再生中
private:
	std::string name;      //モデル名
	std::string directory; //モデルがあるディレクトリパス
	vector<Node> nodes;    //ノード配列

	Node* meshNode;                         //メッシュを持つノード
	vector<VertexPosNormalUvSkin> vertices; //頂点データ配列
	vector<unsigned short> indices;         //頂点インデックス配列

	ComPtr<ID3D12Resource> vertBuff;          //頂点バッファ
	ComPtr<ID3D12Resource> indexBuff;         //インデックスバッファ
	ComPtr<ID3D12Resource> texBuff;           //テクスチャバッファ
	D3D12_VERTEX_BUFFER_VIEW vbView;          //頂点バッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView;           //インデックスバッファビュー
	ComPtr<ID3D12Resource> constBuff;         //定数バッファ
	ComPtr<ID3D12Resource> constBuffSkin;     //定数バッファ(スキン)
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV用デスクリプタヒープ

	Vector3 ambient;                  //アンビエント係数
	Vector3 diffuse;                  //ディフューズ係数
	DirectX::TexMetadata metadata;    //テクスチャメタデータ
	DirectX::ScratchImage scratchImg; //スクラッチイメージ

	vector<Bone> bones; //ボーン配列

	FbxScene* fbxScene; //FBXシーン

public: // メンバ関数
	Model();
	~Model();

	// 初期化
	void Init();
	// 更新処理
	int Update();
	// 描画
	void Draw();
	// 終了処理
	void Finalize();

	// 各種バッファの生成
	HRESULT CreateBuffers();
	HRESULT CreateConstBuffer();

	// アニメーション開始
	void PlayAnimation();

	const Matrix4& GetModelTransform() { return meshNode->globalTransform; }
	vector<Bone>& GetBones() { return bones; }
	FbxScene* GetFbxScene() { return fbxScene; }
};
