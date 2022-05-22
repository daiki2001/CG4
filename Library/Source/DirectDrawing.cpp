#include "./Header/DirectDrawing.h"
#include "./Header/DirectXInit.h"
#include <string>
#include "./Math/Collision/BaseCollider.h"
#include "./Math/Collision/CollisionManager.h"
#include "./Header/SafeDelete.h"

#define EoF -1 // End of Function

CommonData::CommonData() :
	pipelinestate{},
	rootsignature(nullptr)
{
	matProjection[CommonData::Projection::ORTHOGRAPHIC] = DirectX::XMMatrixIdentity();
	matProjection[CommonData::Projection::PERSPECTIVE] = DirectX::XMMatrixIdentity();
}

Object::Object() :
	polygonData(-1),
	constBuff{},
	position(0.0f, 0.0f, 0.0f),
	rotation(DirectX::XMMatrixIdentity()),
	scale(1.0f, 1.0f, 1.0f),
	color(1.0f, 1.0f, 1.0f, 1.0f),
	matWorld(DirectX::XMMatrixIdentity()),
	parent(nullptr),
	name(nullptr),
	collider(nullptr)
{
}

Object::~Object()
{
	if (collider != nullptr)
	{
		// コライダーの登録を解除する
		CollisionManager::Get()->RemoveCollider(collider);
	}

	safe_delete(parent);
	safe_delete(collider);
}

void Object::Init()
{
	name = typeid(*this).name();
}

void Object::Update()
{
	// 当たり判定更新
	if (collider != nullptr)
	{
		collider->Update();
	}
}

void Object::SetCollider(BaseCollider* collider)
{
	collider->SetObject(this);
	this->collider = collider;
	// コライダーを登録する
	CollisionManager::Get()->AddCollider(collider);
	// コライダーを更新しておく
	collider->Update();
}

/*static変数の初期化*/
ID3D12Device* DirectDrawing::dev = DirectXInit::GetDevice();
ID3D12GraphicsCommandList* DirectDrawing::cmdList = DirectXInit::GetCommandList();
CommonData DirectDrawing::spriteData = {};
DirectDrawing::vector<Sprite> DirectDrawing::sprite = {};
DirectDrawing::vector<DirectDrawing::IndexData> DirectDrawing::spriteIndex = {};
size_t DirectDrawing::blendMode = BLENDMODE_NOBLEND;
bool DirectDrawing::isDepthWriteBan = false;

void DirectDrawing::DataClear()
{
	sprite.clear();
	spriteIndex.clear();
	vertices.clear();
	obj.clear();
	objIndex.clear();
	objSubset.clear();
}

DirectDrawing::DirectDrawing() :
	vertices{},
	vertMap(nullptr),
	obj{},
	objIndex{},
	basicDescHeap{},
	srvHandle{},
	nullBufferCount(0),
	vsBlob{},
	psBlob{},
	errorBlob{},
	rootSigBlob{},
	nearClip(0.1f),
	farClip(1000.0f)
{
	using namespace DirectX;

	float winW = static_cast<float>(DirectXInit::GetInstance()->windowWidth);
	float winH = static_cast<float>(DirectXInit::GetInstance()->windowHeight);

	for (size_t i = 0; i < sizeof(objectData) / sizeof(objectData[0]); i++)
	{
		if (i >= sizeof(materialData) / sizeof(materialData[0]))
		{
			break;
		}

		objectData[i] = {};
		objectData[i].matProjection[CommonData::Projection::ORTHOGRAPHIC] =
			XMMatrixOrthographicOffCenterLH(0.0f, winW, winH, 0.0f, 0.0f, 1.0f);
		objectData[i].matProjection[CommonData::Projection::PERSPECTIVE] =
			XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), winW / winH, 0.1f, 1000.0f);

		materialData[i] = objectData[i];
	}

	spriteData = {};
	spriteData.matProjection[CommonData::Projection::ORTHOGRAPHIC] =
		XMMatrixOrthographicOffCenterLH(0.0f, winW, winH, 0.0f, 0.0f, 1.0f);

	Init();
}

DirectDrawing::~DirectDrawing()
{
	DataClear();
}

HRESULT DirectDrawing::Init()
{
	HRESULT hr = S_FALSE;

	dev = DirectXInit::GetDevice();

	hr = DrawingInit();
	if (FAILED(hr))
	{
		return hr;
	}

	hr = MaterialInit();
	if (FAILED(hr))
	{
		return hr;
	}

	hr = SpriteDrawingInit();
	if (FAILED(hr))
	{
		return hr;
	}

	CreateNullConstant(XMFLOAT3(), XMMATRIX(), XMFLOAT3(1, 1, 1));

	return hr;
}

HRESULT DirectDrawing::DrawingInit()
{
	HRESULT hr;

	if (isDrawingInit == true)
	{
		return S_OK;
	}

	isDrawingInit = true;

	// 頂点シェーダの読み込みとコンパイル
	hr = D3DCompileFromFile(
		L"./lib/Shaders/ObjectVS.hlsl", //シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,               //インクルード可能にする
		"main",                                          //エントリーポイント名
		"vs_5_0",                                        //シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //デバッグ用設定
		0,
		&vsBlob,
		&errorBlob);

	if (FAILED(hr))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	hr = D3DCompileFromFile(
		L"./lib/Shaders/ObjectPS.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errorBlob);

	if (FAILED(hr))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		// xyz座標
		{
			"POSITION",                                 //セマンティック名
			0,                                          //同じセマンティック名が複数あるときに使うインデックス
			DXGI_FORMAT_R32G32B32_FLOAT,                //要素数とビット数を表す
			0,                                          //入力スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,               //データのオフセット値
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, //入力データ種別
			0                                           //一度に描画するインスタンス数
		},
		// 法線ベクトル
		{
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		// uv座標
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	// デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV = {}; //デスクリプタテーブルの設定(シェーダリソース)
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[2]{}; //ルートパラメータの設定
	rootparams[0].InitAsConstantBufferView(0);
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_0(
		_countof(rootparams),
		rootparams,
		1,
		&samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	hr = D3DX12SerializeVersionedRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob
	);
	if (FAILED(hr))
	{
		return hr;
	}

	for (size_t i = 0; i < sizeof(objectData) / sizeof(objectData[0]); i++)
	{
		hr = dev->CreateRootSignature(
			0,
			rootSigBlob->GetBufferPointer(),
			rootSigBlob->GetBufferSize(),
			IID_PPV_ARGS(&objectData[i].rootsignature)
		);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	for (size_t i = 0; i < sizeof(objectData) / sizeof(objectData[0]); i++)
	{
		for (size_t j = 0; j < sizeof(CommonData::pipelinestate) / sizeof(CommonData::pipelinestate[0]); j++)
		{
			// グラフィックスパイプライン設定
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

			// 頂点シェーダ、ピクセルシェーダをパイプラインに設定
			gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
			gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
			// サンプルマスクとラスタライザステートの設定
			gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; //標準設定
			gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //背面カリングしない
			// ブレンドデスクの設定
			D3D12_RENDER_TARGET_BLEND_DESC& blendDesc = gpipeline.BlendState.RenderTarget[0];
			blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; //標準設定

			switch (j)
			{
			case BLENDMODE_NOBLEND:
				/*ノーブレンド用の設定*/
				blendDesc.BlendEnable = false; //ブレンドを無効にする
				break;
			case BLENDMODE_ALPHA:
				/*αブレンド用の設定*/
				blendDesc.BlendOp = D3D12_BLEND_OP_ADD;          //加算
				blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;      //ソースのアルファ値
				blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; //1.0f - ソースのアルファ値
				goto BaseBlendState;
			case BLENDMODE_ADD:
				/*加算合成用の設定*/
				blendDesc.BlendOp = D3D12_BLEND_OP_ADD; //加算
				blendDesc.SrcBlend = D3D12_BLEND_ONE;   //ソースの値を 100% 使う
				blendDesc.DestBlend = D3D12_BLEND_ONE;  //デストの値を 100% 使う
				goto BaseBlendState;
			case BLENDMODE_SUB:
				/*減算合成用の設定*/
				blendDesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; //デストからソースを減算
				blendDesc.SrcBlend = D3D12_BLEND_ONE;            //ソースの値を 100% 使う
				blendDesc.DestBlend = D3D12_BLEND_ONE;           //デストの値を 100% 使う
				goto BaseBlendState;
			case BLENDMODE_INV:
				/*色反転用の設定*/
				blendDesc.BlendOp = D3D12_BLEND_OP_ADD;          //加算
				blendDesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR; //1.0f - デストカラーの値
				blendDesc.DestBlend = D3D12_BLEND_ZERO;          //使わない
				goto BaseBlendState;
			default:
BaseBlendState:
				/*共通設定*/
				blendDesc.BlendEnable = true;                //ブレンドを有効にする
				blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; //加算
				blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;   //ソースの値を 100% 使う
				blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO; //デストの値を   0% 使う
				break;
			}
			// デプスステンシルステートの設定
			gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			if (i == 1)
			{
				gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; //デプスの上書き禁止
			}
			gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT; //深度値フォーマット
			// 頂点レイアウトの設定
			gpipeline.InputLayout.pInputElementDescs = inputLayout;
			gpipeline.InputLayout.NumElements = _countof(inputLayout);
			// 図形の形状を三角形に設定
			gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			// その他の設定
			gpipeline.NumRenderTargets = 1; //描画対象は1つ
			gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0~255指定のRGBA
			gpipeline.SampleDesc.Count = 1; //1ピクセルにつき1回サンプリング

			// パイプラインにルートシグネチャをセット
			gpipeline.pRootSignature = objectData[i].rootsignature.Get();

			hr = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&objectData[i].pipelinestate[j]));
		}
	}

	return hr;
}

HRESULT DirectDrawing::MaterialInit()
{
	HRESULT hr;

	if (isMaterialInit == true)
	{
		return S_OK;
	}

	isMaterialInit = true;

	// 頂点シェーダの読み込みとコンパイル
	hr = D3DCompileFromFile(
		L"./lib/Shaders/MaterialVS.hlsl", //シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,               //インクルード可能にする
		"main",                                          //エントリーポイント名
		"vs_5_0",                                        //シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //デバッグ用設定
		0,
		&vsBlob,
		&errorBlob);

	if (FAILED(hr))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	hr = D3DCompileFromFile(
		L"./lib/Shaders/MaterialPS.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errorBlob);

	if (FAILED(hr))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		// xyz座標
		{
			"POSITION",                                 //セマンティック名
			0,                                          //同じセマンティック名が複数あるときに使うインデックス
			DXGI_FORMAT_R32G32B32_FLOAT,                //要素数とビット数を表す
			0,                                          //入力スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,               //データのオフセット値
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, //入力データ種別
			0                                           //一度に描画するインスタンス数
		},
		// 法線ベクトル
		{
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		// uv座標
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	// デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV = {}; //デスクリプタテーブルの設定(シェーダリソース)
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[3]{}; //ルートパラメータの設定
	rootparams[0].InitAsConstantBufferView(0);
	rootparams[1].InitAsConstantBufferView(1);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_0(
		_countof(rootparams),
		rootparams,
		1,
		&samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	// バージョン自動判定のシリアライズ
	hr = D3DX12SerializeVersionedRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob
	);
	if (FAILED(hr))
	{
		return hr;
	}

	for (size_t i = 0; i < sizeof(materialData) / sizeof(materialData[0]); i++)
	{
		hr = dev->CreateRootSignature(
			0,
			rootSigBlob->GetBufferPointer(),
			rootSigBlob->GetBufferSize(),
			IID_PPV_ARGS(&materialData[i].rootsignature)
		);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	for (size_t i = 0; i < sizeof(materialData) / sizeof(materialData[0]); i++)
	{
		for (size_t j = 0; j < sizeof(CommonData::pipelinestate) / sizeof(CommonData::pipelinestate[0]); j++)
		{
			// グラフィックスパイプライン設定
			D3D12_GRAPHICS_PIPELINE_STATE_DESC materialPipeline = {};

			// 頂点シェーダ、ピクセルシェーダをパイプラインに設定
			materialPipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
			materialPipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
			// サンプルマスクとラスタライザステートの設定
			materialPipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; //標準設定
			materialPipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			//materialPipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //背面カリングしない
#pragma region SetBlendState
			D3D12_RENDER_TARGET_BLEND_DESC& materialBlendDesc(materialPipeline.BlendState.RenderTarget[0]);
			materialBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; //標準設定

			switch (j)
			{
			case BLENDMODE_NOBLEND:
				/*ノーブレンド用の設定*/
				materialBlendDesc.BlendEnable = false; //ブレンドを無効にする
				break;
			case BLENDMODE_ALPHA:
				/*αブレンド用の設定*/
				materialBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;          //加算
				materialBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;      //ソースのアルファ値
				materialBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; //1.0f - ソースのアルファ値
				goto MaterialBaseBlend;
			case BLENDMODE_ADD:
				/*加算合成用の設定*/
				materialBlendDesc.BlendOp = D3D12_BLEND_OP_ADD; //加算
				materialBlendDesc.SrcBlend = D3D12_BLEND_ONE;   //ソースの値を 100% 使う
				materialBlendDesc.DestBlend = D3D12_BLEND_ONE;  //デストの値を 100% 使う
				goto MaterialBaseBlend;
			case BLENDMODE_SUB:
				/*減算合成用の設定*/
				materialBlendDesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; //デストからソースを減算
				materialBlendDesc.SrcBlend = D3D12_BLEND_ONE;            //ソースの値を 100% 使う
				materialBlendDesc.DestBlend = D3D12_BLEND_ONE;           //デストの値を 100% 使う
				goto MaterialBaseBlend;
			case BLENDMODE_INV:
				/*色反転用の設定*/
				materialBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;          //加算
				materialBlendDesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR; //1.0f - デストカラーの値
				materialBlendDesc.DestBlend = D3D12_BLEND_ZERO;          //使わない
				goto MaterialBaseBlend;
			default:
MaterialBaseBlend:
				/*共通設定*/
				materialBlendDesc.BlendEnable = true;                //ブレンドを有効にする
				materialBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; //加算
				materialBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;   //ソースの値を 100% 使う
				materialBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO; //デストの値を   0% 使う
				break;
			}
#pragma endregion
			// デプスステンシルステートの設定
			materialPipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			if (i == 1)
			{
				materialPipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; //デプスの上書き禁止
			}
			materialPipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT; //深度値フォーマット
			// 頂点レイアウトの設定
			materialPipeline.InputLayout.pInputElementDescs = inputLayout;
			materialPipeline.InputLayout.NumElements = _countof(inputLayout);
			// 図形の形状を三角形に設定
			materialPipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			// その他の設定
			materialPipeline.NumRenderTargets = 1; //描画対象は1つ
			materialPipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0~255指定のRGBA
			materialPipeline.SampleDesc.Count = 1; //1ピクセルにつき1回サンプリング

			// パイプラインにルートシグネチャをセット
			materialPipeline.pRootSignature = materialData[i].rootsignature.Get();

			hr = dev->CreateGraphicsPipelineState(&materialPipeline, IID_PPV_ARGS(&materialData[i].pipelinestate[j]));
		}
	}

	return hr;
}

HRESULT DirectDrawing::SpriteDrawingInit()
{
	HRESULT hr;

	if (isSpriteDrawingInit == true)
	{
		return S_OK;
	}

	isSpriteDrawingInit = true;

	// 頂点シェーダの読み込みとコンパイル
	hr = D3DCompileFromFile(
		L"./lib/Shaders/SpriteVS.hlsl",       //シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,               //インクルード可能にする
		"main",                                          //エントリーポイント名
		"vs_5_0",                                        //シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //デバッグ用設定
		0,
		&vsBlob,
		&errorBlob);

	if (FAILED(hr))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	hr = D3DCompileFromFile(
		L"./lib/Shaders/SpritePS.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errorBlob);

	if (FAILED(hr))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		// xyz座標
		{
			"POSITION",                                 //セマンティック名
			0,                                          //同じセマンティック名が複数あるときに使うインデックス
			DXGI_FORMAT_R32G32B32_FLOAT,                //要素数とビット数を表す
			0,                                          //入力スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,               //データのオフセット値
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, //入力データ種別
			0                                           //一度に描画するインスタンス数
		},
		// uv座標
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	// デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV = {}; //デスクリプタテーブルの設定(シェーダリソース)
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[2]{}; //ルートパラメータの設定
	rootparams[0].InitAsConstantBufferView(0);
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init_1_0(
		_countof(rootparams),
		rootparams,
		1,
		&samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	hr = D3DX12SerializeVersionedRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob
	);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&spriteData.rootsignature)
	);
	if (FAILED(hr))
	{
		return hr;
	}

	for (size_t i = 0; i < sizeof(CommonData::pipelinestate) / sizeof(CommonData::pipelinestate[0]); i++)
	{
		// グラフィックスパイプライン設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC spritePipeline = {};

		// 頂点シェーダ、ピクセルシェーダをパイプラインに設定
		spritePipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
		spritePipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
		// サンプルマスクとラスタライザステートの設定
		spritePipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; //標準設定
		spritePipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		spritePipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //背面カリングしない
#pragma region SetBlendState
		D3D12_RENDER_TARGET_BLEND_DESC& spriteBlendDesc = spritePipeline.BlendState.RenderTarget[0];
		spriteBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; //標準設定

		switch (i)
		{
		case BLENDMODE_NOBLEND:
			/*ノーブレンド用の設定*/
			spriteBlendDesc.BlendEnable = false; //ブレンドを無効にする
			break;
		case BLENDMODE_ALPHA:
			/*αブレンド用の設定*/
			spriteBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;          //加算
			spriteBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;      //ソースのアルファ値
			spriteBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; //1.0f - ソースのアルファ値
			goto SpriteBaseBlend;
		case BLENDMODE_ADD:
			/*加算合成用の設定*/
			spriteBlendDesc.BlendOp = D3D12_BLEND_OP_ADD; //加算
			spriteBlendDesc.SrcBlend = D3D12_BLEND_ONE;   //ソースの値を 100% 使う
			spriteBlendDesc.DestBlend = D3D12_BLEND_ONE;  //デストの値を 100% 使う
			goto SpriteBaseBlend;
		case BLENDMODE_SUB:
			/*減算合成用の設定*/
			spriteBlendDesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; //デストからソースを減算
			spriteBlendDesc.SrcBlend = D3D12_BLEND_ONE;            //ソースの値を 100% 使う
			spriteBlendDesc.DestBlend = D3D12_BLEND_ONE;           //デストの値を 100% 使う
			goto SpriteBaseBlend;
		case BLENDMODE_INV:
			/*色反転用の設定*/
			spriteBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;          //加算
			spriteBlendDesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR; //1.0f - デストカラーの値
			spriteBlendDesc.DestBlend = D3D12_BLEND_ZERO;          //使わない
			goto SpriteBaseBlend;
		default:
SpriteBaseBlend:
			/*共通設定*/
			spriteBlendDesc.BlendEnable = true;                //ブレンドを有効にする
			spriteBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; //加算
			spriteBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;   //ソースの値を 100% 使う
			spriteBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO; //デストの値を   0% 使う
			break;
		}
#pragma endregion
		// デプスステンシルステートの設定
		spritePipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		spritePipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; //常に上書き
		spritePipeline.DepthStencilState.DepthEnable = false;                      //深度テストをしない
		spritePipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT; //深度値フォーマット
		// 頂点レイアウトの設定
		spritePipeline.InputLayout.pInputElementDescs = inputLayout;
		spritePipeline.InputLayout.NumElements = _countof(inputLayout);
		// 図形の形状を三角形に設定
		spritePipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// その他の設定
		spritePipeline.NumRenderTargets = 1; //描画対象は1つ
		spritePipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0~255指定のRGBA
		spritePipeline.SampleDesc.Count = 1; //1ピクセルにつき1回サンプリング

		// パイプラインにルートシグネチャをセット
		spritePipeline.pRootSignature = spriteData.rootsignature.Get();

		hr = dev->CreateGraphicsPipelineState(&spritePipeline, IID_PPV_ARGS(&spriteData.pipelinestate[i]));
	}

	return hr;
}

int DirectDrawing::CreateVertexAndIndexBuffer()
{
	HRESULT hr = S_FALSE;

	// 'vertexNum'が0以下なら生成せずにリターンする
	if (vertices.size() <= 0 || vertices[vertices.size() - 1].vertices.size() <= 0)
	{
		return EoF;
	}

#pragma region VertexBuffer

	// 頂点データ一つ分のサイズ * 頂点データの要素数
	UINT sizeVB;
	sizeVB = static_cast<UINT>(sizeof(Vertex) * vertices[vertices.size() - 1].vertices.size());

	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertices[vertices.size() - 1].vertBuff));
	if (FAILED(hr))
	{
		return EoF;
	}

	vertices[vertices.size() - 1].vbView.BufferLocation = vertices[vertices.size() - 1].vertBuff->GetGPUVirtualAddress();
	vertices[vertices.size() - 1].vbView.SizeInBytes = sizeVB;
	vertices[vertices.size() - 1].vbView.StrideInBytes = sizeof(Vertex);

#pragma endregion //VertexBuffer

	if (vertices[vertices.size() - 1].indices.size() <= 0)
	{
		for (unsigned short i = 0; i < vertices[vertices.size() - 1].vertices.size(); i++)
		{
			vertices[vertices.size() - 1].indices.push_back(i);
		}
	}

#pragma region IndexBuffer

	//インデックスデータ全体のサイズ
	UINT sizeIB;
	sizeIB = static_cast<UINT>(sizeof(unsigned short) * vertices[vertices.size() - 1].indices.size());

	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertices[vertices.size() - 1].indexBuff));
	if (FAILED(hr))
	{
		return EoF;
	}

	// GPU上のバッファに対応した仮想メモリを取得
	static unsigned short* indexMap = nullptr;
	hr = vertices[vertices.size() - 1].indexBuff->Map(0, nullptr, (void**)&indexMap);

	// 全インデックスに対して
	for (size_t i = 0; i < vertices[vertices.size() - 1].indices.size(); i++)
	{
		indexMap[i] = vertices[vertices.size() - 1].indices[i]; //インデックスをコピー
	}

	// 繋がりを解除
	vertices[vertices.size() - 1].indexBuff->Unmap(0, nullptr);

	vertices[vertices.size() - 1].ibView.BufferLocation = vertices[vertices.size() - 1].indexBuff->GetGPUVirtualAddress();
	vertices[vertices.size() - 1].ibView.Format = DXGI_FORMAT_R16_UINT;
	vertices[vertices.size() - 1].ibView.SizeInBytes = sizeIB;

#pragma endregion //IndexBuffer

#pragma region NormalVector

	for (size_t i = 0; i < vertices[vertices.size() - 1].indices.size() / 3; i++)
	{
		using namespace DirectX;

		// 三角形のインデックスを取り出して、一時的な変数に入れる
		unsigned short temp0 = vertices[vertices.size() - 1].indices[i * 3 + 0];
		unsigned short temp1 = vertices[vertices.size() - 1].indices[i * 3 + 1];
		unsigned short temp2 = vertices[vertices.size() - 1].indices[i * 3 + 2];
		// 三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[vertices.size() - 1].vertices[temp0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[vertices.size() - 1].vertices[temp1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[vertices.size() - 1].vertices[temp2].pos);
		// p0→p1ベクトル、p0→p2ベクトルを計算(ベクトルの減算)
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		// 外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);
		// 正規化
		normal = XMVector3Normalize(normal);
		// 求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[vertices.size() - 1].vertices[temp0].normal, normal);
		XMStoreFloat3(&vertices[vertices.size() - 1].vertices[temp1].normal, normal);
		XMStoreFloat3(&vertices[vertices.size() - 1].vertices[temp2].normal, normal);
	}

	hr = vertices[vertices.size() - 1].vertBuff->Map(0, nullptr, (void**)&vertMap);

	// 全頂点に対して
	for (UINT i = 0; i < vertices[vertices.size() - 1].vertices.size(); i++)
	{
		vertMap[i] = vertices[vertices.size() - 1].vertices[i]; //座標をコピー
	}

	// マップを解除
	vertices[vertices.size() - 1].vertBuff->Unmap(0, nullptr);

#pragma endregion //NormalVector

	return static_cast<int>(vertices.size() - 1);
}

HRESULT DirectDrawing::CreateConstBuffer(int* objIndex)
{
	HRESULT hr = S_FALSE;

	obj.push_back({});

	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xFF) & ~0xFF), //リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&obj[obj.size() - 1].constBuff));
	if (FAILED(hr))
	{
		return hr;
	}
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(MaterialConstBufferData) + 0xFF) & ~0xFF), //リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&obj[obj.size() - 1].materialConstBuff));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = BasicDescHeapInit();
	if (FAILED(hr))
	{
		return hr;
	}

	// 定数バッファビュー生成
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = obj[obj.size() - 1].constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(obj[obj.size() - 1].constBuff->GetDesc().Width);
	dev->CreateConstantBufferView(
		&cbvDesc,
		CD3DX12_CPU_DESCRIPTOR_HANDLE(
			basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
			*objIndex,
			dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		)
	);

	*objIndex = static_cast<int>(obj.size() - 1);
	return hr;
}

int DirectDrawing::CreateNullConstant(const XMFLOAT3& pos, const XMMATRIX& rota, const XMFLOAT3& scale)
{
	obj.push_back({});

	using namespace DirectX;

	XMMATRIX rot = XMMatrixIdentity();

	obj[obj.size() - 1].position = pos;
	obj[obj.size() - 1].rotation = rota;
	obj[obj.size() - 1].scale = scale;
	obj[obj.size() - 1].matWorld = XMMatrixIdentity();

	obj[obj.size() - 1].matWorld *= XMMatrixScaling(
		obj[obj.size() - 1].scale.x,
		obj[obj.size() - 1].scale.y,
		obj[obj.size() - 1].scale.z);
	obj[obj.size() - 1].matWorld *= obj[obj.size() - 1].rotation;
	obj[obj.size() - 1].matWorld *= XMMatrixTranslation(
		obj[obj.size() - 1].position.x,
		obj[obj.size() - 1].position.y,
		obj[obj.size() - 1].position.z);

	nullBufferCount++;

	return static_cast<int>(obj.size() - 1);
}

void DirectDrawing::UpdataConstant(
	const XMFLOAT3& pos, const DirectX::XMMATRIX& rota, const XMFLOAT3& scale,
	const DirectX::XMFLOAT4& color, const int& objectIndex, const int& polygonData,
	Object* parent)
{
	if ((objectIndex < 0 || static_cast<size_t>(objectIndex) >= obj.size()) ||
		(polygonData < -1 || (polygonData != -1 && static_cast<size_t>(polygonData) >= vertices.size()))/* ||
		(parent < -1 || (parent != -1 && static_cast<size_t>(parent) >= obj.size()))*/)
	{
		return;
	}

	using namespace DirectX;
	using namespace Engine::Math;

	bool dirtyFlag = false; //ダーティフラグ

	// 頂点情報が違ったら更新
	if (obj[objectIndex].polygonData != polygonData)
	{
		obj[objectIndex].polygonData = polygonData;
	}

	// 座標が違ったら更新し、ダーティフラグをtrueにする
	if (obj[objectIndex].position.x != pos.x ||
		obj[objectIndex].position.y != pos.y ||
		obj[objectIndex].position.z != pos.z)
	{
		obj[objectIndex].position = pos;
		dirtyFlag = true;
	}

	// 回転行列が違ったら更新し、ダーティフラグをtrueにする
	if ((obj[objectIndex].rotation.r[0].m128_f32[0] != rota.r[0].m128_f32[0]) ||
		(obj[objectIndex].rotation.r[0].m128_f32[1] != rota.r[0].m128_f32[1]) ||
		(obj[objectIndex].rotation.r[0].m128_f32[2] != rota.r[0].m128_f32[2]) ||
		(obj[objectIndex].rotation.r[0].m128_f32[3] != rota.r[0].m128_f32[3]) ||
		(obj[objectIndex].rotation.r[1].m128_f32[0] != rota.r[0].m128_f32[0]) ||
		(obj[objectIndex].rotation.r[1].m128_f32[1] != rota.r[0].m128_f32[1]) ||
		(obj[objectIndex].rotation.r[1].m128_f32[2] != rota.r[0].m128_f32[2]) ||
		(obj[objectIndex].rotation.r[1].m128_f32[3] != rota.r[0].m128_f32[3]) ||
		(obj[objectIndex].rotation.r[2].m128_f32[0] != rota.r[0].m128_f32[0]) ||
		(obj[objectIndex].rotation.r[2].m128_f32[1] != rota.r[0].m128_f32[1]) ||
		(obj[objectIndex].rotation.r[2].m128_f32[2] != rota.r[0].m128_f32[2]) ||
		(obj[objectIndex].rotation.r[2].m128_f32[3] != rota.r[0].m128_f32[3]) ||
		(obj[objectIndex].rotation.r[3].m128_f32[0] != rota.r[0].m128_f32[0]) ||
		(obj[objectIndex].rotation.r[3].m128_f32[1] != rota.r[0].m128_f32[1]) ||
		(obj[objectIndex].rotation.r[3].m128_f32[2] != rota.r[0].m128_f32[2]) ||
		(obj[objectIndex].rotation.r[3].m128_f32[3] != rota.r[0].m128_f32[3]))
	{
		obj[objectIndex].rotation = rota;
		dirtyFlag = true;
	}

	// スケールが違ったら更新し、ダーティフラグをtrueにする
	if (obj[objectIndex].scale.x != scale.x ||
		obj[objectIndex].scale.y != scale.y ||
		obj[objectIndex].scale.z != scale.z)
	{
		obj[objectIndex].scale = scale;
		dirtyFlag = true;
	}

	// 親の要素番号が違ったら更新し、ダーティフラグをtrueにする
	if (obj[objectIndex].parent != nullptr)
	{
		obj[objectIndex].parent = parent;
		dirtyFlag = true;
	}

	// ダーティフラグがtrueだったら、ワールド行列を更新
	if (dirtyFlag == true)
	{
		obj[objectIndex].matWorld = XMMatrixIdentity();
		obj[objectIndex].matWorld *= XMMatrixScaling(
			obj[objectIndex].scale.x,
			obj[objectIndex].scale.y,
			obj[objectIndex].scale.z);
		obj[objectIndex].matWorld *= obj[objectIndex].rotation;
		obj[objectIndex].matWorld *= XMMatrixTranslation(
			obj[objectIndex].position.x,
			obj[objectIndex].position.y,
			obj[objectIndex].position.z);

		if (obj[objectIndex].parent != nullptr)
		{
			obj[objectIndex].matWorld *= parent->matWorld;
		}
	}

	// 色が違ったら更新
	if (obj[objectIndex].color != color)
	{
		obj[objectIndex].color = color;
	}
}

int DirectDrawing::CreateSprite()
{
	using namespace DirectX;

	HRESULT hr = S_FALSE;
	sprite.push_back({});

	static SpriteVertex vert[] = {
		{{}, { 0.0f, 1.0f }},
		{{}, { 0.0f, 0.0f }},
		{{}, { 1.0f, 1.0f }},
		{{}, { 1.0f, 0.0f }},
	};

	// 頂点バッファの生成
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vert)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sprite[sprite.size() - 1].vertBuff));
	if (FAILED(hr))
	{
		return EoF;
	}

	// 頂点バッファへのデータ転送
	SpriteVertex* vertexMap = nullptr;
	hr = sprite[sprite.size() - 1].vertBuff->Map(0, nullptr, (void**)&vertexMap);
	memcpy(vertexMap, vert, sizeof(vert));
	sprite[sprite.size() - 1].vertBuff->Unmap(0, nullptr);

	// 頂点バッファビューの生成
	sprite[sprite.size() - 1].vbView.BufferLocation = sprite[sprite.size() - 1].vertBuff->GetGPUVirtualAddress();
	sprite[sprite.size() - 1].vbView.SizeInBytes = sizeof(vert);
	sprite[sprite.size() - 1].vbView.StrideInBytes = sizeof(SpriteVertex);

	// 定数バッファの生成
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xFF) & ~0xFF), //リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sprite[sprite.size() - 1].constBuff));
	if (FAILED(hr))
	{
		return EoF;
	}

	// 定数バッファにデータ転送
	SpriteConstBufferData* constMap = nullptr;
	hr = sprite[sprite.size() - 1].constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = XMFLOAT4(1, 1, 1, 1); //色指定(RGBA)
	constMap->mat = spriteData.matProjection[CommonData::Projection::ORTHOGRAPHIC]; //平行投影行列
	sprite[sprite.size() - 1].constBuff->Unmap(0, nullptr);

	hr = BasicDescHeapInit();
	if (FAILED(hr))
	{
		return EoF;
	}

	return static_cast<int>(sprite.size() - 1);
}

HRESULT DirectDrawing::BasicDescHeapInit()
{
	HRESULT hr;

	if (isBasicDescHeapInit == true)
	{
		return S_OK;
	}
	else
	{
		isBasicDescHeapInit = true;

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{}; //設定構造体

		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //シェーダから見える
		descHeapDesc.NumDescriptors = static_cast<UINT>(50000); //定数バッファの数
		// デスクリプタヒープの生成
		hr = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

		return hr;
	}
}

void DirectDrawing::BaseDrawGraphics()
{
	cmdList = DirectXInit::GetCommandList();

	// ビューポート領域の設定
	cmdList->RSSetViewports(
		1,
		&CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<float>(DirectXInit::GetInstance()->windowWidth),
			static_cast<float>(DirectXInit::GetInstance()->windowHeight)
		)
	);

	// シザー矩形の設定
	cmdList->RSSetScissorRects(
		1,
		&CD3DX12_RECT(
			0,
			0,
			DirectXInit::GetInstance()->windowWidth,
			DirectXInit::GetInstance()->windowHeight
		)
	);
}

void DirectDrawing::BaseDrawSpriteGraphics()
{
	cmdList = DirectXInit::GetCommandList();

	// ビューポート領域の設定
	cmdList->RSSetViewports(
		1,
		&CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<float>(DirectXInit::GetInstance()->windowWidth),
			static_cast<float>(DirectXInit::GetInstance()->windowHeight)
		)
	);

	// シザー矩形の設定
	cmdList->RSSetScissorRects(
		1,
		&CD3DX12_RECT(
			0,
			0,
			DirectXInit::GetInstance()->windowWidth,
			DirectXInit::GetInstance()->windowHeight
		)
	);

	cmdList->SetPipelineState(spriteData.pipelinestate[blendMode].Get());
	cmdList->SetGraphicsRootSignature(spriteData.rootsignature.Get());

	// プリミティブ形状の設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

int DirectDrawing::SetDrawBlendMode(int blendMode)
{
	if (blendMode < BLENDMODE_NOBLEND || blendMode > BLENDMODE_INV)
	{
		return EoF;
	}

	this->blendMode = blendMode;

	return static_cast<int>(this->blendMode);
}

HRESULT DirectDrawing::SetNearFar(float nearClip, float farClip)
{
	using namespace DirectX;

	if (nearClip <= 0.0f || farClip <= 0.0f || nearClip == farClip)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	this->nearClip = nearClip;
	this->farClip = farClip;

	for (size_t i = 0; i < sizeof(objectData) / sizeof(objectData[0]); i++)
	{
		objectData[i].matProjection[CommonData::Projection::ORTHOGRAPHIC] = XMMatrixOrthographicOffCenterLH(
			0.0f,
			static_cast<float>(DirectXInit::GetInstance()->windowWidth),
			static_cast<float>(DirectXInit::GetInstance()->windowHeight),
			0.0f,
			this->nearClip, //前端
			this->farClip   //奥端
		);

		objectData[i].matProjection[CommonData::Projection::PERSPECTIVE] = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(60.0f),                      //上下画角60度
			static_cast<float>(DirectXInit::GetInstance()->windowWidth) /
			static_cast<float>(DirectXInit::GetInstance()->windowHeight), //アスペクト比
			this->nearClip, //前端
			this->farClip   //奥端
		);
	}

	for (size_t i = 0; i < sizeof(materialData) / sizeof(materialData[0]); i++)
	{
		materialData[i].matProjection[CommonData::Projection::ORTHOGRAPHIC] = XMMatrixOrthographicOffCenterLH(
			0.0f,
			static_cast<float>(DirectXInit::GetInstance()->windowWidth),
			static_cast<float>(DirectXInit::GetInstance()->windowHeight),
			0.0f,
			this->nearClip, //前端
			this->farClip   //奥端
		);

		materialData[i].matProjection[CommonData::Projection::PERSPECTIVE] = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(60.0f),                    //上下画角60度
			static_cast<float>(DirectXInit::GetInstance()->windowWidth) /
			static_cast<float>(DirectXInit::GetInstance()->windowHeight), //アスペクト比
			this->nearClip, //前端
			this->farClip   //奥端
		);
	}

	return S_OK;
}
