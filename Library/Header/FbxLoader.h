#pragma once
#include "fbxsdk.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <string>
#include "Model.h"

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
	FbxLoader() = default;
	// privateなデストラクタ（シングルトンパターン）
	~FbxLoader() = default;
	// コピーコンストラクタを禁止（シングルトンパターン）
	FbxLoader(const FbxLoader& obj) = delete;
	// コピー代入演算子を禁止（シングルトンパターン）
	void operator=(const FbxLoader& obj) = delete;

private: // エイリアス
	using string = std::string;

public: // 定数

public: // メンバ関数
	// 初期化
	void Init();
	// 終了処理
	void Finalize();

	// ファイルからFBXモデル読み込み
	void LoadModelFromFile(const string& modelPath);
private:
	// 再帰的にノード構成を解析
	void ParseNodeRecursive(Model* model, FbxNode* fbxNode, Node* parent = nullptr);
	// メッシュ読み取り
	void ParseMesh(Model* model, FbxNode* fbxNode);

	// 頂点座標読み取り
	void ParseMeshVertices(Model* model, FbxMesh* fbxMesh);
	// 面情報読み取り
	void ParseMeshFaces(Model* model, FbxMesh* fbxMesh);
	// マテリアル読み取り
	void ParseMaterial(Model* model, FbxNode* fbxNode);
	// テクスチャ読み取り
	void LoadTexture(Model* model, const string& fullpath);

private: // メンバ変数
	ID3D12Device* device = nullptr; //D3D12デバイス
	FbxManager* fbxManger = nullptr; //FBXマネージャー
	FbxImporter* fbxImporter = nullptr; //FBXインポーター
};