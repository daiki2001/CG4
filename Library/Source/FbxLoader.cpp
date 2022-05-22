#include "./Header/FbxLoader.h"
#include "./Header/DirectXInit.h"
#include "./Header/EngineGeneral.h"

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
	//assert(fbxManger == nullptr);
	ErrorLog("fbxMangerを再初期化しようとしています。\n", fbxManger != nullptr);

	device = DirectXInit::GetDevice();

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
	using namespace Engine::General;

	size_t foundExtension = modelPath.find_last_of(".");
	string directoryPath = ExtractDirectory(modelPath);
	string modelName = modelPath.substr(modelPath.size(), foundExtension - modelPath.size());

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
	model->directory = directoryPath;
	// FBXノードの数を取得
	int nodeCount = fbxScene->GetNodeCount();
	model->nodes.reserve(nodeCount);
	// ルートノードから順に解析してモデルに流し込む
	ParseNodeRecursive(model.get(), fbxScene->GetRootNode());
	// バッファの生成
	model->CreateBuffers();
	// FBXシーンの解放
	fbxScene->Destroy();
}

void FbxLoader::ParseNodeRecursive(Model* model, FbxNode* fbxNode, Node* parent)
{
	using namespace Engine::Math;

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

	// FBXノードのメッシュ情報を解析
	FbxNodeAttribute* fbxNodeAttribute = fbxNode->GetNodeAttribute();

	if (fbxNodeAttribute)
	{
		if (fbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			model->meshNode = &node;
			ParseMesh(model, fbxNode);
		}
	}

	//子ノードに対して再帰呼び出し
	for (int i = 0; i < fbxNode->GetChildCount(); i++)
	{
		ParseNodeRecursive(model, fbxNode->GetChild(i), &node);
	}
}

void FbxLoader::ParseMesh(Model* model, FbxNode* fbxNode)
{
	// ノードのメッシュを取得
	FbxMesh* fbxMesh = fbxNode->GetMesh();

	ParseMeshVertices(model, fbxMesh);
	ParseMeshFaces(model, fbxMesh);
	ParseMaterial(model, fbxNode);
}

void FbxLoader::ParseMeshVertices(Model* model, FbxMesh* fbxMesh)
{
	auto& vertices = model->vertices;

	// 頂点座標データの数
	const int controlPointsCount = fbxMesh->GetControlPointsCount();
	// 必要数だけ頂点データ配列を確保
	Model::VertexPosNormalUv vert{};
	model->vertices.resize(controlPointsCount, vert);

	// FBXメッシュの頂点座標配列を取得
	FbxVector4* pCoord = fbxMesh->GetControlPoints();

	// FBXメッシュの全頂点座標をモデル内の配列にコピーする
	for (int i = 0; i < controlPointsCount; i++)
	{
		Model::VertexPosNormalUv& vertex = vertices[i];
		// 座標のコピー
		vertex.pos.x = static_cast<float>(pCoord[i][0]);
		vertex.pos.y = static_cast<float>(pCoord[i][1]);
		vertex.pos.z = static_cast<float>(pCoord[i][2]);
	}
}

void FbxLoader::ParseMeshFaces(Model* model, FbxMesh* fbxMesh)
{
	auto& vertices = model->vertices;
	auto& indices = model->indices;

	// 1ファイルに複数メッシュのモデルは非対応
	assert(indices.size() == 0);
	// 面の数
	const int polygonCount = fbxMesh->GetPolygonCount();
	// UVデータの数
	const int textureUVCount = fbxMesh->GetTextureUVCount();
	// UV名リスト
	FbxStringList uvNames;
	fbxMesh->GetUVSetNames(uvNames);

	// 面ごとの情報読み取り
	for (int i = 0; i < polygonCount; i++)
	{
		// 面を構成する頂点の数を取得
		const int polygonSize = fbxMesh->GetPolygonSize(i);
		assert(polygonSize <= 4);

		// 1頂点ずつ処理
		for (int j = 0; j < polygonSize; j++)
		{
			// FBX頂点配列のインデックス
			int index = fbxMesh->GetPolygonVertex(i, j);
			assert(index >= 0);

			// 頂点法線読み込み
			Model::VertexPosNormalUv& vertex = vertices[index];
			FbxVector4 normal;
			if (fbxMesh->GetPolygonVertexNormal(i, j, normal))
			{
				vertex.normal.x = static_cast<float>(normal[0]);
				vertex.normal.y = static_cast<float>(normal[1]);
				vertex.normal.z = static_cast<float>(normal[2]);
			}

			// テクスチャUV読み込み
			if (textureUVCount > 0)
			{
				FbxVector2 uvs;
				bool lUnmappadUV = false;
				// 0番決め打ちで読み込み
				if (fbxMesh->GetPolygonVertexUV(i, j, uvNames[0], uvs, lUnmappadUV))
				{
					vertex.uv.x = static_cast<float>(uvs[0]);
					vertex.uv.y = static_cast<float>(uvs[1]);
				}
			}

			// インデックス配列に頂点インデックス追加
			if (j < 3)
			{
				indices.push_back(index);
			}
			else
			{
				int index2 = indices[indices.size() - 1];
				int index3 = index;
				int index0 = indices[indices.size() - 3];
				indices.push_back(index2);
				indices.push_back(index3);
				indices.push_back(index0);
			}
		}
	}
}

void FbxLoader::ParseMaterial(Model* model, FbxNode* fbxNode)
{
	using namespace Engine::General;

	const int materialCount = fbxNode->GetMaterialCount();
	if (materialCount > 0)
	{
		// 先頭のマテリアルを取得
		FbxSurfaceMaterial* material = fbxNode->GetMaterial(0);
		// テクスチャを読み込んだかどうかのフラグ
		bool textureLoaded = false;

		if (material)
		{
			// FbxSurfaceLambertクラスかどうかを調べる
			if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				FbxSurfaceLambert* lambert = static_cast<FbxSurfaceLambert*>(material);

				// 環境光係数
				FbxPropertyT<FbxDouble3> ambint = lambert->Ambient;
				model->ambient.x = static_cast<float>(ambint.Get()[0]);
				model->ambient.y = static_cast<float>(ambint.Get()[1]);
				model->ambient.z = static_cast<float>(ambint.Get()[2]);

				// 拡散反射光係数
				FbxPropertyT<FbxDouble3> diffuse = lambert->Diffuse;
				model->diffuse.x = static_cast<float>(diffuse.Get()[0]);
				model->diffuse.y = static_cast<float>(diffuse.Get()[1]);
				model->diffuse.z = static_cast<float>(diffuse.Get()[2]);
			}

			// ディフューズテクスチャを取り出す
			const FbxProperty diffuseProperty =
				material->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if (diffuseProperty.IsValid())
			{
				const FbxFileTexture* texture = diffuseProperty.GetSrcObject<FbxFileTexture>();
				if (texture)
				{
					const char* filepath = texture->GetFileName();
					string name = ExtractFileName(filepath);

					LoadTexture(model, model->directory + name);
					textureLoaded = true;
				}
			}
		}
		// テクスチャがない場合は白テクスチャを貼る
		if (textureLoaded == false)
		{
			LoadTexture(model, libraryDirectory + defaultTextureFileName);
		}
	}
}

void FbxLoader::LoadTexture(Model* model, const string& fullpath)
{
	using namespace DirectX;

	HRESULT hr = S_FALSE;
	// WICテクスチャのロード
	TexMetadata& metadata = model->metadata;
	ScratchImage& scratchImg = model->scratchImg;
	// ユニコード文字列に変換
	wchar_t wfilepath[128];
	MultiByteToWideChar(CP_ACP, 0, fullpath.c_str(), -1, wfilepath, _countof(wfilepath));
	hr = LoadFromWICFile(wfilepath, WIC_FLAGS_NONE, &metadata, scratchImg);
	ErrorLog("テクスチャの読み込み失敗\n", FAILED(hr));
}
