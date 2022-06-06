#pragma once
#include <string>
#include <vector>
#include "./Math/EngineMath.h"
#include <d3d12.h>
#include <wrl.h>
#include <DirectXTex.h>
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
public: // フレンドクラス
	friend class FbxLoader;

public: // サブクラス
	// 頂点データ構造体
	struct VertexPosNormalUv
	{
		Engine::Math::Vector3 pos;    //xyz座標
		Engine::Math::Vector3 normal; //法線ベクトル
		DirectX::XMFLOAT2 uv;         //uv座標
	};
	// 定数バッファ用データ構造体
	struct ConstBufferData
	{
		Engine::Math::Matrix4 viewProj;  //ビュープロジェクション行列
		Engine::Math::Matrix4 world;     //ワールド行列
		Engine::Math::Vector3 cameraPos; //カメラ座標
	};

private: // エイリアス
	using Vector3 = Engine::Math::Vector3;
	using Matrix4 = Engine::Math::Matrix4;
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	template<class T> using vector = std::vector<T>;

public: // 静的メンバ関数
	static HRESULT CreateGraphicsPipeline();

private: // 静的メンバ変数
	static ID3D12Device* dev;

	static ComPtr<ID3D12RootSignature> rootSignature; //ルートシグネチャ
	static ComPtr<ID3D12PipelineState> pipelineState; //パイプラインステート

public: // メンバ関数
	Model();

	void Init();
	int Update();
	void Draw();

	// 各種バッファの生成
	HRESULT CreateBuffers();
	HRESULT CreateConstBuffer();

	// 各種バッファの生成
	const Matrix4& GetModelTransform() { return meshNode->globalTransform; }

private: // メンバ変数
	std::string name;      //モデル名
	std::string directory; //モデルがあるディレクトリパス
	vector<Node> nodes;    //ノード配列

	Node* meshNode;                     //メッシュを持つノード
	vector<VertexPosNormalUv> vertices; //頂点データ配列
	vector<unsigned short> indices;     //頂点インデックス配列

	ComPtr<ID3D12Resource> vertBuff;          //頂点バッファ
	ComPtr<ID3D12Resource> indexBuff;         //インデックスバッファ
	ComPtr<ID3D12Resource> texBuff;           //テクスチャバッファ
	D3D12_VERTEX_BUFFER_VIEW vbView;          //頂点バッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView;           //インデックスバッファビュー
	ComPtr<ID3D12Resource> constBuff;         //定数バッファ
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV用デスクリプタヒープ

	Vector3 ambient;                  //アンビエント係数
	Vector3 diffuse;                  //ディフューズ係数
	DirectX::TexMetadata metadata;    //テクスチャメタデータ
	DirectX::ScratchImage scratchImg; //スクラッチイメージ

	Vector3 pos;   //ローカル座標
	Matrix4 rota;  //回転行列
	Vector3 scale; //スケール
	Matrix4 world; //ワールド行列
};
