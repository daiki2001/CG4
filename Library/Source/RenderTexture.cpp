#include "./Header/RenderTexture.h"
#include <d3dx12.h>
#include "./Header/DirectXInit.h"

float RenderTexture::clearColor[4] = { 0.1f, 0.875f, 0.875f, 1.0f };

RenderTexture::RenderTexture() :
	texBuff{},
	descHeapSRV{},
	descHeapRTV{}
{
}

RenderTexture::~RenderTexture()
{
}

RenderTexture* RenderTexture::Get()
{
	static RenderTexture instance = {};
	return &instance;
}

HRESULT RenderTexture::CreateRenderTexture()
{
	HRESULT hr = S_FALSE;
	DirectXInit* w = DirectXInit::GetInstance();
	auto dev = DirectXInit::GetDevice();

	// テクスチャリソース設定
	D3D12_RESOURCE_DESC texResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		static_cast<UINT64>(w->windowWidth),
		static_cast<UINT>(w->windowHeight),
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	for (size_t i = 0; i < 2; i++)
	{
		// テクスチャバッファの生成
		hr = dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
									 D3D12_MEMORY_POOL_L0),
			D3D12_HEAP_FLAG_NONE,
			&texResDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //テクスチャ用指定
			&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clearColor),
			IID_PPV_ARGS(&texBuff[i])
		);
		if (FAILED(hr))
		{
			return hr;
		}
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
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// デスクリプタヒープにSRV作成
	dev->CreateShaderResourceView(
		texBuff[0].Get(),
		&srvDesc,
		descHeapSRV->GetCPUDescriptorHandleForHeapStart()
	);

	return hr;
}

HRESULT RenderTexture::CreateRTV()
{
	HRESULT hr = S_FALSE;
	auto dev = DirectXInit::GetDevice();

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
		texBuff[0].Get(),
		nullptr,
		descHeapRTV->GetCPUDescriptorHandleForHeapStart()
	);

	return hr;
}

HRESULT RenderTexture::CreateMultiRTV()
{
	HRESULT hr = S_FALSE;
	auto dev = DirectXInit::GetDevice();

	// RTV用デスクリプタヒープを設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc = {};
	rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHeapDesc.NumDescriptors = 2;
	// RTV用デスクリプタヒープを生成
	hr = dev->CreateDescriptorHeap(&rtvDescHeapDesc, IID_PPV_ARGS(&descHeapMultiRTV));
	if (FAILED(hr))
	{
		return hr;
	}

	for (int i = 0; i < 2; i++)
	{
		// レンダーターゲットビューの生成
		dev->CreateRenderTargetView(
			texBuff[i].Get(),
			nullptr,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(
				descHeapMultiRTV->GetCPUDescriptorHandleForHeapStart(),
				i,
				dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			)
		);
	}

	return hr;
}
