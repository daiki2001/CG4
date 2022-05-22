#pragma once
#include <string>
#include <vector>
#include "./Math/EngineMath.h"
#include <d3d12.h>
#include <wrl.h>
#include <DirectXTex.h>

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

private: // エイリアス
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	template<class T> using vector = std::vector<T>;

private: // 静的メンバ変数
	static ID3D12Device* dev;

public: // メンバ関数
	Model();

	void Init();
	void Draw();

	// 各種バッファの生成
	int CreateBuffers();

	// 各種バッファの生成
	const Engine::Math::Matrix4& GetModelTransform() { return meshNode->globalTransform; }

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
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV用デスクリプタヒープ

	Engine::Math::Vector3 ambient;    //アンビエント係数
	Engine::Math::Vector3 diffuse;    //ディフューズ係数
	DirectX::TexMetadata metadata;    //テクスチャメタデータ
	DirectX::ScratchImage scratchImg; //スクラッチイメージ
};
