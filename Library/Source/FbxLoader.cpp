#include "./Header/FbxLoader.h"
#include "./Header/DirectXInit.h"
#include "./Header/Error.h"

FbxLoader* FbxLoader::GetInstance()
{
	static FbxLoader instance;
	return &instance;
}

void FbxLoader::Init()
{
	// 再初期化チェック
	assert(fbxManger == nullptr);
	//ErrorLog("fbxMangerを再初期化しようとしています。\n", fbxManger == nullptr);

	device = DirectXInit::GetInstance()->dev.Get();

	// FBXマネージャーの生成
	fbxManger = FbxManager::Create();

	// FBXマネージャーの入出力設定
	FbxIOSettings* ios = FbxIOSettings::Create(fbxManger, IOSROOT);
	fbxManger->SetIOSettings(ios);

	// FBXインポータの生成
	fbxImporter = FbxImporter::Create(fbxManger, "");
}

void FbxLoader::Finalize()
{
	// 各種FBXインスタンスの破棄
	fbxImporter->Destroy();
	fbxManger->Destroy();
}

void FbxLoader::LoadModelFromFile(const string& modelPath)
{
	// ファイル名を指定してFBXファイルを読み込む
	if (!fbxImporter->Initialize(modelPath.c_str(), -1, fbxManger->GetIOSettings()))
	{
		assert(0);
	}

	// シーン生成
	FbxScene* fbxScene = FbxScene::Create(fbxManger, "fbxScene");

	// ファイルからロードしたFBXの情報をシーンにインポート
	fbxImporter->Import(fbxScene);
}
