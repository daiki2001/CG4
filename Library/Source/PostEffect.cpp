#include "./Header/PostEffect.h"
#include "./Header/DirectXInit.h"
#include "./Header/Error.h"

const float PostEffect::shaderClearColor[4] = { 1.0f, 0.5f, 0.0f, 1.0f };

PostEffect::PostEffect() :
	LoadTex(),
	position(0.0f, 0.0f, 0.0f),
	angle(0.0f),
	matWorld(Engine::Math::Identity()),
	color(1.0f, 1.0f, 1.0f, 1.0f),
	descHeapSRV{},
	depthBuff{},
	descHeapRTV{},
	descHeapDSV{},
	graphHandle(-1),
	spriteCount(size_t(-1))
{
}

int PostEffect::Init()
{
	using namespace Engine;

	HRESULT hr = S_FALSE;

	int size = CreateSprite();
	if (size == FUNCTION_ERROR)
	{
		return FUNCTION_ERROR;
	}

	spriteIndex.push_back({ size, 0 });
	spriteCount = spriteIndex.size() - 1;

	enum Corner { LB, LT, RB, RT };

	SpriteVertex vert[] = {
		{{}, { 0.0f, 1.0f }},
		{{}, { 0.0f, 0.0f }},
		{{}, { 1.0f, 1.0f }},
		{{}, { 1.0f, 0.0f }},
	};

	float left = 0.0f;
	float right = 100.0f;
	float top = 0.0f;
	float bottom = 100.0f;

	vert[LB].pos = { left, bottom, 0.0f };
	vert[LT].pos = { left, top, 0.0f };
	vert[RB].pos = { right, bottom, 0.0f };
	vert[RT].pos = { right, top, 0.0f };

	// 頂点バッファへのデータ転送
	SpriteVertex* vertexMap = nullptr;
	sprite[spriteIndex[spriteCount].constant].vertBuff->Map(0, nullptr, (void**)&vertexMap);
	memcpy(vertexMap, vert, sizeof(vert));
	sprite[spriteIndex[spriteCount].constant].vertBuff->Unmap(0, nullptr);

	// テクスチャリソース設定
	D3D12_RESOURCE_DESC texResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		static_cast<UINT64>(DirectXInit::GetInstance()->windowWidth),
		static_cast<UINT>(DirectXInit::GetInstance()->windowHeight),
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	auto* dev = DirectXInit::GetDevice();

	// テクスチャバッファの生成
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
								 D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texResDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //テクスチャ用指定
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, shaderClearColor),
		IID_PPV_ARGS(&textrueData[0].texbuff)
	);
	if (FAILED(hr))
	{
		return FUNCTION_ERROR;
	}

	// テクスチャを赤クリア
	{
		// 画素数
		const UINT pixleCount = DirectXInit::GetInstance()->windowWidth *
			DirectXInit::GetInstance()->windowHeight;
		// 画像1行分のデータサイズ
		const UINT rowPitch = sizeof(UINT) * DirectXInit::GetInstance()->windowWidth;
		// 画像全体のデータサイズ
		const UINT depthPitch = rowPitch * DirectXInit::GetInstance()->windowHeight;
		// 画像イメージ
		UINT* img = new UINT[pixleCount];
		for (UINT i = 0; i < pixleCount; i++)
		{
			img[i] = 0xff0000ff;
		}

		// テクスチャバッファにデータ転送
		hr = textrueData[0].texbuff->WriteToSubresource(
			0,
			nullptr,
			img,
			rowPitch,
			depthPitch
		);
		if (FAILED(hr))
		{
			return hr;
		}
		delete[] img;
	}

	// SRV用デスクリプタヒープを設定
	D3D12_DESCRIPTOR_HEAP_DESC srvDescHeapDesc = {};
	srvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescHeapDesc.NumDescriptors = 1;
	// SRV用デスクリプタヒープを生成
	hr = dev->CreateDescriptorHeap(&srvDescHeapDesc, IID_PPV_ARGS(&descHeapSRV));
	if (FAILED(hr))
	{
		return hr;
	}

	// シェーダーリソースビューの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textrueData[0].texbuff->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// デスクリプタヒープにSRV作成
	dev->CreateShaderResourceView(
		textrueData[0].texbuff.Get(),
		&srvDesc,
		descHeapSRV->GetCPUDescriptorHandleForHeapStart()
	);

	// RTV用デスクリプタヒープを設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc = {};
	rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHeapDesc.NumDescriptors = 1;
	// RTV用デスクリプタヒープを生成
	hr = dev->CreateDescriptorHeap(&rtvDescHeapDesc, IID_PPV_ARGS(&descHeapRTV));
	if (FAILED(hr))
	{
		return hr;
	}

	// レンダーターゲットビューの生成
	dev->CreateRenderTargetView(
		textrueData[0].texbuff.Get(),
		nullptr,
		descHeapRTV->GetCPUDescriptorHandleForHeapStart()
	);

	// リソース設定
	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		DirectXInit::GetInstance()->windowWidth,
		DirectXInit::GetInstance()->windowHeight,
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
		return hr;
	}

	// DSV用デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescHeapDesc = {};
	dsvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescHeapDesc.NumDescriptors = 1;
	// DSV用デスクリプタヒープ作成
	hr = dev->CreateDescriptorHeap(&dsvDescHeapDesc, IID_PPV_ARGS(&descHeapDSV));
	if (FAILED(hr))
	{
		return hr;
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

	static auto* cmdList = DirectXInit::GetCommandList();
	static bool isInit = false;

	if (isInit == false)
	{
		isInit = true;
	}

	if (spriteIndex.size() == 0)
	{
		return FUNCTION_ERROR;
	}

#pragma region GraphicsCommand

	IndexData& index = spriteIndex[spriteCount];

	matWorld = Math::Identity();
	matWorld *=Math::rotateZ(angle * Math::degree);
	matWorld *= Math::translate(Vector3(
		sprite[index.constant].pos.x,
		sprite[index.constant].pos.y,
		0.0f
	));

	//if (parent != -1)
	//{
	//	sprite[index.constant].matWorld *= sprite[parent].matWorld;
	//}

	SpriteConstBufferData* constMap = nullptr;
	HRESULT hr = sprite[index.constant].constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(hr))
	{
		constMap->color = color;
		constMap->mat = matWorld * spriteData.matProjection[CommonData::Projection::ORTHOGRAPHIC];
		sprite[index.constant].constBuff->Unmap(0, nullptr);
	}

	cmdList->SetPipelineState(spriteData.pipelinestate[blendMode].Get());
	cmdList->SetGraphicsRootSignature(spriteData.rootsignature.Get());

	// プリミティブ形状の設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// デスクリプタヒープをセット
	ID3D12DescriptorHeap* ppHeaps[] = { descHeapSRV.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット
	cmdList->SetGraphicsRootConstantBufferView(0, sprite[index.constant].constBuff->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(1, descHeapSRV->GetGPUDescriptorHandleForHeapStart());

	// 頂点バッファの設定
	cmdList->IASetVertexBuffers(0, 1, &sprite[index.constant].vbView);
	// 描画コマンド
	cmdList->DrawInstanced(4, 1, 0, 0);

#pragma endregion

	return index.constant;
}

int PostEffect::PreDraw()
{
	static auto* cmdList = DirectXInit::GetCommandList();

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		textrueData[0].texbuff.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	));

	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvH = descHeapRTV->GetCPUDescriptorHandleForHeapStart();
	// 深度ステンシルビュー用ディスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH = descHeapDSV->GetCPUDescriptorHandleForHeapStart();
	// レンダーターゲットをセット
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

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

	// 画面クリア
	cmdList->ClearRenderTargetView(rtvH, shaderClearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	return 0;
}

int PostEffect::PostDraw()
{
	static auto* cmdList = DirectXInit::GetCommandList();

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		textrueData[0].texbuff.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	));

	return 0;
}
