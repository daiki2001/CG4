#include "./Header/FbxLoader.h"
#include "./Header/DirectXInit.h"
#include "./Header/Error.h"
#include <memory>

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
	size_t foundDirectory = modelPath.find_last_of("/\\") + 1;
	size_t foundExtension = modelPath.find_last_of(".");
	string directoryPath = modelPath.substr(0, foundDirectory);
	string modelName = modelPath.substr(foundDirectory, foundExtension - foundDirectory);

	// ファイル名を指定してFBXファイルを読み込む
	if (!fbxImporter->Initialize(modelPath.c_str(), -1, fbxManger->GetIOSettings()))
	{
		assert(0);
	}

	// シーン生成
	FbxScene* fbxScene = FbxScene::Create(fbxManger, "fbxScene");

	// ファイルからロードしたFBXの情報をシーンにインポート
	fbxImporter->Import(fbxScene);

	// モデル生成
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->name = modelName;
	// FBXノードの数を取得
	int nodeCount = fbxScene->GetNodeCount();
	model->nodes.reserve(nodeCount);
	// ルートノードから順に解析してモデルに流し込む
	ParseNodeRecursive(model.get(), fbxScene->GetRootNode());
	// FBXシーンの解放
	fbxScene->Destroy();
}

void FbxLoader::ParseNodeRecursive(Model* model, FbxNode* fbxNode, Node* parent)
{
	using namespace EngineMath;

	// ノード名を取得
	string name = fbxNode->GetName();

	// モデルにノードを追加
	model->nodes.emplace_back();
	Node& node = model->nodes.back();
	// ノード名を取得
	node.name = fbxNode->GetName();

	// FBXノードのローカル移動情報
	fbxsdk::FbxDouble3 translation = fbxNode->LclTranslation;
	fbxsdk::FbxDouble3 rotation = fbxNode->LclRotation;
	fbxsdk::FbxDouble3 scaling = fbxNode->LclScaling;

	// 形式変換して代入
	node.position = {
		static_cast<float>(translation[0]),
		static_cast<float>(translation[1]),
		static_cast<float>(translation[2])
	};
	Vector3 rota = {
		static_cast<float>(rotation[0]),
		static_cast<float>(rotation[1]),
		static_cast<float>(rotation[2])
	};
	node.scaling = {
		static_cast<float>(scaling[0]),
		static_cast<float>(scaling[1]),
		static_cast<float>(scaling[2])
	};

	// 回転角をDegreeからRadianに変換
	rota *= degree;

	// 回転行列の計算
	node.rotation = rotateZ(rota.z);
	node.rotation *= rotateX(rota.x);
	node.rotation *= rotateY(rota.y);

	// 平行移動、スケール行列の計算
	Matrix4 matTranslation = translate(node.position);
	Matrix4 matScaling = scale(node.scaling);

	// ローカル変形行列の計算
	node.transform = Identity();
	node.transform *= matScaling;
	node.transform *= node.rotation;
	node.transform *= matTranslation;

	// グローバル変形行列の計算
	node.globalTransform = node.transform;
	if (parent)
	{
		node.parent = parent;
		node.globalTransform *= parent->globalTransform;
	}

	//子ノードに対して再帰呼び出し
	for (int i = 0; i < fbxNode->GetChildCount(); i++)
	{
		ParseNodeRecursive(model, fbxNode->GetChild(i), &node);
	}
}
