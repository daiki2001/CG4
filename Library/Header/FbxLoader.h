#pragma once
#include "fbxsdk.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <string>
#include "Model.h"

#include "./Math/EngineMath.h"

class FbxLoader
{
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>インスタンス</returns>
	static FbxLoader* GetInstance();

private:
	// privateなコンストラクタ（シングルトンパターン）
	FbxLoader();
	// privateなデストラクタ（シングルトンパターン）
	~FbxLoader() = default;
	// コピーコンストラクタを禁止（シングルトンパターン）
	FbxLoader(const FbxLoader& obj) = delete;
	// コピー代入演算子を禁止（シングルトンパターン）
	void operator=(const FbxLoader& obj) = delete;

private: // エイリアス
	using string = std::string;

public: // 定数
	
public: // 静的メンバ関数
	static Engine::Math::Matrix4 ConvertMatrixFromFbx(const FbxAMatrix& src);

private: // メンバ変数
	ID3D12Device* device; //D3D12デバイス

	FbxManager* fbxManger;    //FBXマネージャー
	FbxImporter* fbxImporter; //FBXインポーター

	std::vector<std::unique_ptr<Model>> models; //モデルの配列

public: // メンバ関数
	// 初期化
	void Init();
	// 終了処理
	void Finalize();

	// ファイルからFBXモデル読み込み
	int LoadModelFromFile(const string& modelPath);

	// モデルの取得
	Model* GetModel(int index);
private:
	// 再帰的にノード構成を解析
	void ParseNodeRecursive(Model* model, FbxNode* fbxNode, Node* parent = nullptr);
	// メッシュ読み取り
	void ParseMesh(Model* model, FbxNode* fbxNode);

	// 頂点座標読み取り
	void ParseMeshVertices(Model* model, FbxMesh* fbxMesh);
	// 面を構成するデータの読み取り
	void ParseMeshFaces(Model* model, FbxMesh* fbxMesh);
	// マテリアルの読み取り
	void ParseMaterial(Model* model, FbxNode* fbxNode);
	// テクスチャ読み込み
	void LoadTexture(Model* model, const string& fullpath);
	// スキニング情報の読み取り
	int ParseSkin(Model* model, FbxMesh* fbxMesh);
};