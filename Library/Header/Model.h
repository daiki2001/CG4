#pragma once
#include <string>
#include <vector>
#include "./Math/EngineMath.h"
#include <DirectXTex.h>

struct Node
{
	std::string name;                    //名前
	EngineMath::Vector3 position;        //ローカル座標
	EngineMath::Matrix4 rotation;        //ローカル回転行列
	EngineMath::Vector3 scaling;         //ローカルスケール
	EngineMath::Matrix4 transform;       //ローカル変換行列
	EngineMath::Matrix4 globalTransform; //グローバル変換行列
	Node* parent;                        //親ノード

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
		EngineMath::Vector3 pos;    //xyz座標
		EngineMath::Vector3 normal; //法線ベクトル
		DirectX::XMFLOAT2 uv;       //uv座標
	};

private: // エイリアス
	template<class T> using vector = std::vector<T>;

public: // メンバ関数
	Model();

private: // メンバ変数
	std::string name;      //モデル名
	std::string directory; //モデルがあるディレクトリパス
	vector<Node> nodes;    //ノード配列

	Node* meshNode;                     //メッシュを持つノード
	vector<VertexPosNormalUv> vertices; //頂点データ配列
	vector<unsigned short> indices;     //頂点インデックス配列

	EngineMath::Vector3 ambient;      //アンビエント係数
	EngineMath::Vector3 diffuse;      //ディフューズ係数
	DirectX::TexMetadata metadata;    //テクスチャメタデータ
	DirectX::ScratchImage scratchImg; //スクラッチイメージ
};
