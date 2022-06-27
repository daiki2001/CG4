#include "./Header/RenderTexture.h"
#include <d3dx12.h>
#include "./Header/DirectXInit.h"
#include <cassert>

float RenderTexture::clearColor[4] = { 0.1f, 0.875f, 0.875f, 1.0f };

RenderTexture::RenderTexture()
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

HRESULT RenderTexture::CreateRenderTexture(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>* const texBuff,
										   Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>* descHeapSRV, const UINT& texCount)
{
	HRESULT hr = S_FALSE;
	DirectXInit* w = DirectXInit::GetInstance();
	auto dev = DirectXInit::GetDevice();

	texBuff->resize(texCount);

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

	for (size_t i = 0; i < texCount; i++)
	{
		// テクスチャバッファの生成
		hr = dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
									 D3D12_MEMORY_POOL_L0),
			D3D12_HEAP_FLAG_NONE,
			&texResDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //テクスチャ用指定
			&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clearColor),
			IID_PPV_ARGS(&(*texBuff)[i])
		);
		if (FAILED(hr))
		{
			return hr;
		}

		// テクスチャを赤クリア
		{
			// 画素数
			const UINT pixleCount = w->windowWidth * w->windowHeight;
			// 画像一行分のデータサイズ
			const UINT rowPitch = sizeof(UINT) * w->windowWidth;
			// 画像全体のデータサイズ
			const UINT depthPitch = rowPitch * w->windowHeight;
			// 画像イメージ
			UINT* img = new UINT[pixleCount];
			for (size_t j = 0; j < pixleCount; j++)
			{
				img[j] = 0xFF0000FF;
			}

			hr = (*texBuff)[i]->WriteToSubresource(0, nullptr, img, rowPitch, depthPitch);
			if (FAILED(hr))
			{
				assert(0);
				return hr;
			}

			delete[] img;
		}
	}

	// SRV用デスクリプタヒープを設定
	D3D12_DESCRIPTOR_HEAP_DESC srvDescHeapDesc = {};
	srvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescHeapDesc.NumDescriptors = texCount;
	// SRV用デスクリプタヒープを生成
	hr = dev->CreateDescriptorHeap(&srvDescHeapDesc, IID_PPV_ARGS(&(*descHeapSRV)));
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

	for (int i = 0; i < static_cast<int>(texCount); i++)
	{
		// デスクリプタヒープにSRV作成
		dev->CreateShaderResourceView(
			(*texBuff)[i].Get(),
			&srvDesc,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(
				(*descHeapSRV)->GetCPUDescriptorHandleForHeapStart(),
				i,
				dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			));
	}

	return hr;
}

HRESULT RenderTexture::CreateRTV(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>* descHeapRTV,
								 const std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> texBuff)
{
	HRESULT hr = S_FALSE;
	auto dev = DirectXInit::GetDevice();

	// RTV用デスクリプタヒープを設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc = {};
	rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHeapDesc.NumDescriptors = static_cast<int>(texBuff.size());
	// RTV用デスクリプタヒープを生成
	hr = dev->CreateDescriptorHeap(&rtvDescHeapDesc, IID_PPV_ARGS(&(*descHeapRTV)));
	if (FAILED(hr))
	{
		return hr;
	}

	for (int i = 0; i < static_cast<int>(texBuff.size()); i++)
	{
		// レンダーターゲットビューの生成
		dev->CreateRenderTargetView(
			texBuff[i].Get(),
			nullptr,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(
				(*descHeapRTV)->GetCPUDescriptorHandleForHeapStart(),
				i,
				dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			)
		);
	}

	return hr;
}
