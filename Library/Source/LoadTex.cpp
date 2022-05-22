#include "./Header/LoadTex.h"
#include "./Header/Error.h"

TextrueCommon::TextrueCommon() :
	metadata{},
	scratchImg{},
	img{},
	textrueCount(1),
	descHeap{}
{
	noneTextrue[0] = { 1.0f, 1.0f, 1.0f, 1.0f };
}

/*static変数の初期化*/
DirectDrawing::vector<Textrue> LoadTex::textrueData = {};
TextrueCommon LoadTex::texCommonData = {};

LoadTex::LoadTex() :
	DirectDrawing(),
	spriteCount(0)
{
}

LoadTex::~LoadTex()
{
	DataClear();
}

int LoadTex::LoadTextrue(const wchar_t* fileName)
{
	using namespace Engine;

	HRESULT hr = S_FALSE;
	CD3DX12_RESOURCE_DESC texResDesc{};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; //設定構造体

	if (isLoadTextrueInit == false)
	{
		isLoadTextrueInit = true;

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{}; //設定構造体
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //シェーダから見える
		descHeapDesc.NumDescriptors = (UINT)(50000); //テクスチャバッファの数
		// デスクリプタヒープの生成
		hr = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texCommonData.descHeap));
		if (FAILED(hr))
		{
			return functionError;
		}
	}

	if (textrueData.size() <= 0)
	{
		texResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			1,
			1,
			1,
			1
		);

		textrueData.push_back({});
		hr = dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
			D3D12_HEAP_FLAG_NONE,
			&texResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, //テクスチャ用指定
			nullptr,
			IID_PPV_ARGS(&textrueData[0].texbuff)
		);
		if (FAILED(hr))
		{
			return functionError;
		}

		// テクスチャバッファにデータ転送
		hr = textrueData[0].texbuff->WriteToSubresource(
			0,
			nullptr,                   //全領域へコピー
			texCommonData.noneTextrue, //元データアドレス
			sizeof(XMFLOAT4) * 1,      //1ラインサイズ
			sizeof(XMFLOAT4) * 1       //全サイズ
		);
		if (FAILED(hr))
		{
			return functionError;
		}

		// デスクリプタヒープの先頭ハンドル(CPU)を取得
		textrueData[textrueData.size() - 1].cpuDescHandle =
			CD3DX12_CPU_DESCRIPTOR_HANDLE(
				texCommonData.descHeap->GetCPUDescriptorHandleForHeapStart(),
				(INT)(textrueData.size() - 1),
				dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			);

		// デスクリプタヒープの先頭ハンドル(GPU)を取得
		textrueData[textrueData.size() - 1].gpuDescHandle =
			CD3DX12_GPU_DESCRIPTOR_HANDLE(
				texCommonData.descHeap->GetGPUDescriptorHandleForHeapStart(),
				(INT)(textrueData.size() - 1),
				dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			);

		// シェーダリソースビュー設定
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクスチャ
		srvDesc.Texture2D.MipLevels = 1;

		dev->CreateShaderResourceView(
			textrueData[0].texbuff.Get(), //ビューと関連付けるバッファ
			&srvDesc, //テクスチャ設定情報
			textrueData[textrueData.size() - 1].cpuDescHandle
		);
	}
	if (fileName != nullptr)
	{
		texCommonData.textrueCount++;

		// WICテクスチャのロード
		LoadFromWICFile(
			fileName,
			DirectX::WIC_FLAGS_IGNORE_SRGB,
			&texCommonData.metadata,
			texCommonData.scratchImg
		);

		texCommonData.img = texCommonData.scratchImg.GetImage(0, 0, 0);

		texResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			texCommonData.metadata.format,
			texCommonData.metadata.width,
			(UINT)texCommonData.metadata.height,
			(UINT16)texCommonData.metadata.arraySize,
			(UINT16)texCommonData.metadata.mipLevels
		);

		textrueData.push_back({});
		hr = dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
			D3D12_HEAP_FLAG_NONE,
			&texResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, //テクスチャ用指定
			nullptr,
			IID_PPV_ARGS(&textrueData[textrueData.size() - 1].texbuff)
		);
		if (FAILED(hr))
		{
			return functionError;
		}

		// テクスチャバッファにデータ転送
		hr = textrueData[textrueData.size() - 1].texbuff->WriteToSubresource(
			0,
			nullptr,                            //全領域へコピー
			texCommonData.img->pixels,          //元データアドレス
			(UINT)texCommonData.img->rowPitch,  //1ラインサイズ
			(UINT)texCommonData.img->slicePitch //一枚サイズ
		);
		if (FAILED(hr))
		{
			return functionError;
		}

		// デスクリプタヒープの先頭ハンドル(CPU)を取得
		textrueData[textrueData.size() - 1].cpuDescHandle =
			CD3DX12_CPU_DESCRIPTOR_HANDLE(
				texCommonData.descHeap->GetCPUDescriptorHandleForHeapStart(),
				(INT)(textrueData.size() - 1),
				dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			);

		// デスクリプタヒープの先頭ハンドル(GPU)を取得
		textrueData[textrueData.size() - 1].gpuDescHandle =
			CD3DX12_GPU_DESCRIPTOR_HANDLE(
				texCommonData.descHeap->GetGPUDescriptorHandleForHeapStart(),
				(INT)(textrueData.size() - 1),
				dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			);

		// シェーダリソースビュー設定
		srvDesc.Format = texCommonData.metadata.format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクスチャ
		srvDesc.Texture2D.MipLevels = 1;

		dev->CreateShaderResourceView(
			textrueData[textrueData.size() - 1].texbuff.Get(), //ビューと関連付けるバッファ
			&srvDesc, //テクスチャ設定情報
			textrueData[textrueData.size() - 1].cpuDescHandle
		);
	}

	return texCommonData.textrueCount - 1;
}

void LoadTex::DataClear()
{
	textrueData.clear();
	texCommonData = {};
}

int LoadTex::DrawTextureInit()
{
	using namespace Engine;

	int size = 0;

	size = CreateSprite();
	if (size == functionError)
	{
		return functionError;
	}

	if (isDrawTextrueInit == false)
	{
		isDrawTextrueInit = true;

		if (LoadTextrue() == functionError)
		{
			return functionError;
		}
	}

	return size;
}

int LoadTex::DrawTextrue(const float& posX, const float& posY, const float& width, const float& height,
						 const float& angle, const int& graphHandle, const DirectX::XMFLOAT2& anchorpoint,
						 const XMFLOAT4& color, const int& parent)
{
	using namespace DirectX;
	using namespace Engine;

	if ((graphHandle < 0 || (UINT)graphHandle > texCommonData.textrueCount) ||
		(parent < -1 || (parent != -1 && (size_t)parent >= spriteIndex.size())))
	{
		return functionError;
	}

	bool isInit = false;

	if ((size_t)(spriteCount + 1) < spriteIndex.size())
	{
		isInit = true;
	}

	if (isInit == false)
	{
		int size = DrawTextureInit();
		if (size == functionError)
		{
			return functionError;
		}

		spriteIndex.push_back({ size, graphHandle });
	}

	if (spriteIndex.size() == 0)
	{
		return functionError;
	}

	spriteCount++;
	spriteCount %= spriteIndex.size();

#pragma region GraphicsCommand

	BaseDrawSpriteGraphics();

	IndexData& index = spriteIndex[spriteCount];
	index.textrue = graphHandle;

	if (sprite[index.constant].size.x != width ||
		sprite[index.constant].size.y != height ||
		sprite[index.constant].anchorpoint.x != anchorpoint.x ||
		sprite[index.constant].anchorpoint.y != anchorpoint.y)
	{
		sprite[index.constant].size = { width, height };
		sprite[index.constant].anchorpoint = anchorpoint;

		enum Corner { LB, LT, RB, RT };

		SpriteVertex vert[] = {
			{{}, { 0.0f, 1.0f }},
			{{}, { 0.0f, 0.0f }},
			{{}, { 1.0f, 1.0f }},
			{{}, { 1.0f, 0.0f }},
		};

		float left = (0.0f - sprite[index.constant].anchorpoint.x) * sprite[index.constant].size.x;
		float right = (1.0f - sprite[index.constant].anchorpoint.x) * sprite[index.constant].size.x;
		float top = (0.0f - sprite[index.constant].anchorpoint.y) * sprite[index.constant].size.y;
		float bottom = (1.0f - sprite[index.constant].anchorpoint.y) * sprite[index.constant].size.y;

		vert[LB].pos = { left, bottom, 0.0f };
		vert[LT].pos = { left, top, 0.0f };
		vert[RB].pos = { right, bottom, 0.0f };
		vert[RT].pos = { right, top, 0.0f };

		// 頂点バッファへのデータ転送
		SpriteVertex* vertexMap = nullptr;
		sprite[index.constant].vertBuff->Map(0, nullptr, (void**)&vertexMap);
		memcpy(vertexMap, vert, sizeof(vert));
		sprite[index.constant].vertBuff->Unmap(0, nullptr);
	}

	sprite[index.constant].color = color;
	sprite[index.constant].pos = { posX, posY, 0 };
	sprite[index.constant].rotation = angle;

	sprite[index.constant].matWorld = XMMatrixIdentity();
	sprite[index.constant].matWorld *=
		XMMatrixRotationZ(XMConvertToRadians(sprite[index.constant].rotation));
	sprite[index.constant].matWorld *= XMMatrixTranslation(
		sprite[index.constant].pos.x,
		sprite[index.constant].pos.y,
		sprite[index.constant].pos.z);

	if (parent != -1)
	{
		sprite[index.constant].matWorld *= sprite[parent].matWorld;
	}

	SpriteConstBufferData* constMap = nullptr;
	sprite[index.constant].constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = sprite[index.constant].color;
	constMap->mat = sprite[index.constant].matWorld *
		spriteData.matProjection[CommonData::Projection::ORTHOGRAPHIC];
	sprite[index.constant].constBuff->Unmap(0, nullptr);

	// デスクリプタヒープをセット
	ID3D12DescriptorHeap* ppHeaps[] = { texCommonData.descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット
	cmdList->SetGraphicsRootConstantBufferView(0, sprite[index.constant].constBuff->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(1, textrueData[index.textrue].gpuDescHandle);

	// 頂点バッファの設定
	cmdList->IASetVertexBuffers(0, 1, &sprite[index.constant].vbView);
	// 描画コマンド
	cmdList->DrawInstanced(4, 1, 0, 0);

#pragma endregion

	return index.constant;
}

int LoadTex::DrawCutTextrue(const float& posX, const float& posY, const float& width, const float& height,
							const DirectX::XMFLOAT2& texPos, const DirectX::XMFLOAT2& texSize, const float& angle,
							const int& graphHandle, const DirectX::XMFLOAT2& anchorpoint, const XMFLOAT4& color, const int& parent)
{
	using namespace DirectX;
	using namespace Engine;

	if ((graphHandle < 0 || (UINT)graphHandle > texCommonData.textrueCount) ||
		(parent < -1 && (size_t)parent >= spriteIndex.size()))
	{
		return functionError;
	}

	bool isInit = false;

	if ((size_t)(spriteCount + 1) < spriteIndex.size())
	{
		isInit = true;
	}

	if (isInit == false)
	{
		int size = DrawTextureInit();
		if (size == functionError)
		{
			return functionError;
		}

		spriteIndex.push_back({ size, graphHandle });
	}

	if (spriteIndex.size() == 0)
	{
		return functionError;
	}

	spriteCount++;
	spriteCount %= spriteIndex.size();

#pragma region GraphicsCommand

	BaseDrawSpriteGraphics();

	IndexData& index = spriteIndex[spriteCount];
	index.textrue = graphHandle;

	if (sprite[index.constant].size.x != width ||
		sprite[index.constant].size.y != height ||
		sprite[index.constant].texLeftTop.x != texPos.x ||
		sprite[index.constant].texLeftTop.y != texPos.y ||
		sprite[index.constant].texSize.x != texSize.x ||
		sprite[index.constant].texSize.y != texSize.y ||
		sprite[index.constant].anchorpoint.x != anchorpoint.x ||
		sprite[index.constant].anchorpoint.y != anchorpoint.y)
	{
		sprite[index.constant].size = { width, height };
		sprite[index.constant].texLeftTop = texPos;
		sprite[index.constant].texSize = texSize;
		sprite[index.constant].anchorpoint = anchorpoint;

		enum Corner { LB, LT, RB, RT };

		static SpriteVertex vert[] = {
			{{}, { 0.0f, 1.0f }},
			{{}, { 0.0f, 0.0f }},
			{{}, { 1.0f, 1.0f }},
			{{}, { 1.0f, 0.0f }},
		};

		float left = (0.0f - sprite[index.constant].anchorpoint.x) * sprite[index.constant].size.x;
		float right = (1.0f - sprite[index.constant].anchorpoint.x) * sprite[index.constant].size.x;
		float top = (0.0f - sprite[index.constant].anchorpoint.y) * sprite[index.constant].size.y;
		float bottom = (1.0f - sprite[index.constant].anchorpoint.y) * sprite[index.constant].size.y;

		vert[LB].pos = { left, bottom, 0.0f };
		vert[LT].pos = { left, top, 0.0f };
		vert[RB].pos = { right, bottom, 0.0f };
		vert[RT].pos = { right, top, 0.0f };

		// テクスチャデータ取得
		D3D12_RESOURCE_DESC resDesc = textrueData[index.textrue].texbuff->GetDesc();

		float texLeft = sprite[index.constant].texLeftTop.x / resDesc.Width;
		float texRight = (sprite[index.constant].texLeftTop.x + sprite[index.constant].texSize.x) / resDesc.Width;
		float texTop = sprite[index.constant].texLeftTop.y / resDesc.Height;
		float texBottom = (sprite[index.constant].texLeftTop.y + sprite[index.constant].texSize.y) / resDesc.Height;

		vert[LB].uv = { texLeft, texBottom };
		vert[LT].uv = { texLeft, texTop };
		vert[RB].uv = { texRight, texBottom };
		vert[RT].uv = { texRight, texTop };

		// 頂点バッファへのデータ転送
		SpriteVertex* vertexMap = nullptr;
		sprite[index.constant].vertBuff->Map(0, nullptr, (void**)&vertexMap);
		memcpy(vertexMap, vert, sizeof(vert));
		sprite[index.constant].vertBuff->Unmap(0, nullptr);
	}

	sprite[index.constant].color = color;
	sprite[index.constant].pos = { posX, posY, 0 };
	sprite[index.constant].rotation = angle;

	sprite[index.constant].matWorld = XMMatrixIdentity();
	sprite[index.constant].matWorld *=
		XMMatrixRotationZ(XMConvertToRadians(sprite[index.constant].rotation));
	sprite[index.constant].matWorld *=
		XMMatrixTranslation(
			sprite[index.constant].pos.x,
			sprite[index.constant].pos.y,
			sprite[index.constant].pos.z);

	if (parent != -1)
	{
		sprite[index.constant].matWorld *= sprite[parent].matWorld;
	}

	SpriteConstBufferData* constMap = nullptr;
	sprite[index.constant].constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = sprite[index.constant].color;
	constMap->mat = sprite[index.constant].matWorld *
		spriteData.matProjection[CommonData::Projection::ORTHOGRAPHIC];
	sprite[index.constant].constBuff->Unmap(0, nullptr);

	// デスクリプタヒープをセット
	ID3D12DescriptorHeap* ppHeaps[] = { texCommonData.descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット
	cmdList->SetGraphicsRootConstantBufferView(0, sprite[index.constant].constBuff->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(1, textrueData[index.textrue].gpuDescHandle);

	// 頂点バッファの設定
	cmdList->IASetVertexBuffers(0, 1, &sprite[index.constant].vbView);
	// 描画コマンド
	cmdList->DrawInstanced(4, 1, 0, 0);

#pragma endregion

	return index.constant;
}

void LoadTex::LoopEnd()
{
	spriteCount = -1;
}
