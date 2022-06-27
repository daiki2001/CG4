#include "./Header/PostEffect.h"
#include "./Header/DirectXInit.h"
#include "./Header/DirectDrawing.h"
#include "./Header/RenderTexture.h"

#include "./Header/Error.h"

/*シェーダ用*/
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

const float PostEffect::clearColor[4] = { 1.0f, 0.5f, 0.0f, 0.0f };

PostEffect::PostEffect() :
	position(0.0f, 0.0f, 0.0f),
	angle(0.0f),
	matWorld(Engine::Math::Identity()),
	color(1.0f, 1.0f, 1.0f, 1.0f),
	pipelineState{},
	rootSignature{},
	vertBuff{},
	vbView{},
	constBuff{},
	texBuff{},
	descHeapSRV{},
	descHeapRTV{},
	depthBuff{},
	descHeapDSV{}
{
}

int PostEffect::Init()
{
	using namespace Engine;

	HRESULT hr = S_FALSE;
	DirectXInit* w = DirectXInit::GetInstance();
	auto dev = DirectXInit::GetDevice();
	auto renderTex = RenderTexture::Get();

	hr = CreateGraphicsPipelineState();
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	hr = CreateVertexBuffer();
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	// 定数バッファの生成
	hr = CreateConstantBuffer();
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	hr = renderTex->CreateRenderTexture(&texBuff, &descHeapSRV, 2);
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	hr = renderTex->CreateRTV(&descHeapRTV, texBuff);
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	// リソース設定
	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		w->windowWidth,
		w->windowHeight,
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	// リソース生成
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値書き込みに使用
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
		IID_PPV_ARGS(&depthBuff)
	);
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	// DSV用デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescHeapDesc = {};
	dsvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescHeapDesc.NumDescriptors = 1;
	// DSV用デスクリプタヒープ作成
	hr = dev->CreateDescriptorHeap(&dsvDescHeapDesc, IID_PPV_ARGS(&descHeapDSV));
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	// デスクリプタヒープにDSV作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; //深度値フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dev->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		descHeapDSV->GetCPUDescriptorHandleForHeapStart()
	);

	return 0;
}

int PostEffect::Draw()
{
	using namespace DirectX;
	using namespace Engine;

	static bool isInit = false;
	static auto dev = DirectXInit::GetDevice();
	static auto cmdList = DirectXInit::GetCommandList();
	static auto renderTex = RenderTexture::Get();

	if (isInit == false)
	{
		isInit = true;
	}

#pragma region GraphicsCommand

	SpriteConstBufferData* constMap = nullptr;
	HRESULT hr = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(hr))
	{
		constMap->color = color;
		constMap->mat = Math::Identity();
		constBuff->Unmap(0, nullptr);
	}

	cmdList->SetPipelineState(pipelineState.Get());
	cmdList->SetGraphicsRootSignature(rootSignature.Get());

	// プリミティブ形状の設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// デスクリプタヒープをセット
	ID3D12DescriptorHeap* ppHeaps[] = { descHeapSRV.Get()};
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット
	cmdList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(
		1,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(
			descHeapSRV->GetGPUDescriptorHandleForHeapStart(),
			0,
			dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		));
	cmdList->SetGraphicsRootDescriptorTable(
		2,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(
			descHeapSRV->GetGPUDescriptorHandleForHeapStart(),
			1,
			dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		));

	// 頂点バッファの設定
	cmdList->IASetVertexBuffers(0, 1, &vbView);
	// 描画コマンド
	cmdList->DrawInstanced(4, 1, 0, 0);

#pragma endregion

	return hr;
}

int PostEffect::PreDraw()
{
	static DirectXInit* w = DirectXInit::GetInstance();
	static auto dev = DirectXInit::GetDevice();
	static auto cmdList = DirectXInit::GetCommandList();
	static auto renderTex = RenderTexture::Get();

	for (size_t i = 0; i < texBuff.size(); i++)
	{
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			texBuff[i].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtvH[2] = {};
	for (int i = 0; i < texBuff.size(); i++)
	{
		// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
		rtvH[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			descHeapRTV->GetCPUDescriptorHandleForHeapStart(),
			i,
			dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		);
	}
	// 深度ステンシルビュー用ディスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH = descHeapDSV->GetCPUDescriptorHandleForHeapStart();
	// レンダーターゲットをセット
	cmdList->OMSetRenderTargets(_countof(rtvH), rtvH, false, &dsvH);

	// ビューポート領域
	CD3DX12_VIEWPORT viewports[2] = {};
	// シザー矩形
	CD3DX12_RECT scissorRects[2] = {};

	for (size_t i = 0; i < texBuff.size(); i++)
	{
		viewports[i] = CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<float>(w->windowWidth),
			static_cast<float>(w->windowHeight)
		);

		scissorRects[i] = CD3DX12_RECT(
			0,
			0,
			w->windowWidth,
			w->windowHeight
		);
	}

	// ビューポート領域の設定
	cmdList->RSSetViewports(_countof(viewports), viewports);

	// シザー矩形の設定
	cmdList->RSSetScissorRects(_countof(scissorRects), scissorRects);

	// 画面クリア
	for (size_t i = 0; i < texBuff.size(); i++)
	{
		cmdList->ClearRenderTargetView(rtvH[i], clearColor, 0, nullptr);
	}
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	return 0;
}

int PostEffect::PostDraw()
{
	static auto cmdList = DirectXInit::GetCommandList();
	static auto renderTex = RenderTexture::Get();

	for (int i = 0; i < texBuff.size(); i++)
	{
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			texBuff[i].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		));
	}

	return 0;
}

HRESULT PostEffect::CreateGraphicsPipelineState()
{
	HRESULT hr = S_FALSE;
	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;
	ComPtr<ID3DBlob> rootSigBlob;
	ComPtr<ID3DBlob> errorBlob;
	auto dev = DirectXInit::GetDevice();

	// 頂点シェーダの読み込みとコンパイル
	hr = D3DCompileFromFile(
		L"./lib/Shaders/PostEffectTestVS.hlsl", //シェーダファイル名
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
		assert(0);
		return hr;
	}

	// ピクセルシェーダの読み込みとコンパイル
	hr = D3DCompileFromFile(
		L"./lib/Shaders/PostEffectTestPS.hlsl",
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
		assert(0);
		return hr;
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		// xyz座標
		{
			"POSITION",
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

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeline = {};

	// 頂点シェーダ、ピクセルシェーダをパイプラインに設定
	gPipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gPipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	// サンプルマスク
	gPipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; //標準設定
	// ラスタライザステート
	gPipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gPipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //背面カリングしない
	// デプスステンシルステート
	gPipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gPipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; //常に上書き

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; //標準設定
	blendDesc.BlendEnable = true;                //ブレンドを有効にする
	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;          //加算
	blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;      //ソースのアルファ値
	blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; //1.0f - ソースのアルファ値

	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; //加算
	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;   //ソースの値を 100% 使う
	blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO; //デストの値を   0% 使う

	// ブレンドステートの設定
	gPipeline.BlendState.RenderTarget[0] = blendDesc;

	// 深度バッファのフォーマット
	gPipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// 頂点レイアウトの設定
	gPipeline.InputLayout.pInputElementDescs = inputLayout;
	gPipeline.InputLayout.NumElements = _countof(inputLayout);

	// 図形の形状設定(三角形)
	gPipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// その他の設定
	gPipeline.NumRenderTargets = 1; //描画対象は1つ
	gPipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0~255指定のRGBA
	gPipeline.SampleDesc.Count = 1; //1ピクセルにつき1回サンプリング

	// デスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV0 = {};
	descRangeSRV0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //t0レジスタ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV1 = {};
	descRangeSRV1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); //t1レジスタ

	// ルートパラメータ
	CD3DX12_ROOT_PARAMETER rootparams[3] = {};
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV1, D3D12_SHADER_VISIBILITY_ALL);

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);

	// ルートシグネチャの設定
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
		assert(0);
		return hr;
	}

	// ルートシグネチャの生成
	hr = dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)
	);
	if (FAILED(hr))
	{
		assert(0);
		return hr;
	}

	// パイプラインにルートシグネチャをセット
	gPipeline.pRootSignature = rootSignature.Get();

	// グラフィックスパイプラインの生成
	hr = dev->CreateGraphicsPipelineState(&gPipeline, IID_PPV_ARGS(&pipelineState));
	if (FAILED(hr))
	{
		assert(0);
		return hr;
	}

	return hr;
}

HRESULT PostEffect::CreateVertexBuffer()
{
	HRESULT hr = S_FALSE;
	auto dev = DirectXInit::GetDevice();

	SpriteVertex vert[] = {
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ -1.0f, +1.0f, 0.0f }, { 0.0f, 0.0f }},
		{{ +1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }},
		{{ +1.0f, +1.0f, 0.0f }, { 1.0f, 0.0f }},
	};

	// 頂点バッファの生成
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vert)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(hr))
	{
		return hr;
	}

	// 頂点バッファへのデータ転送
	SpriteVertex* vertexMap = nullptr;
	hr = vertBuff->Map(0, nullptr, (void**)&vertexMap);
	if (SUCCEEDED(hr))
	{
		memcpy(vertexMap, vert, sizeof(vert));
		vertBuff->Unmap(0, nullptr);
	}

	// 頂点バッファビューの生成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vert);
	vbView.StrideInBytes = sizeof(SpriteVertex);

	return hr;
}

HRESULT PostEffect::CreateConstantBuffer()
{
	HRESULT hr = S_FALSE;
	auto dev = DirectXInit::GetDevice();

	// 定数バッファの生成
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xFF) & ~0xFF), //リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	if (FAILED(hr))
	{
		return hr;
	}

	// 定数バッファにデータ転送
	SpriteConstBufferData* constMap = nullptr;
	hr = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(hr))
	{
		constMap->color = color;
		constMap->mat = Engine::Math::Identity();
		constBuff->Unmap(0, nullptr);
	}

	return hr;
}
