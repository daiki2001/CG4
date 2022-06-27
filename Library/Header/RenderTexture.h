#pragma once
#include <d3d12.h>
#include <wrl.h>

class RenderTexture final
{
private: //エイリアス
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public: //シングルトン化
	static RenderTexture* Get();
private:
	RenderTexture();
	RenderTexture(const RenderTexture&) = delete;
	~RenderTexture();
	RenderTexture operator=(const RenderTexture&) = delete;

public: //静的メンバ変数
	static float clearColor[4];

private: //メンバ変数
	ComPtr<ID3D12Resource> texBuff[2];        //テクスチャバッファ
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV用のデスクリプタヒープ

	ComPtr<ID3D12DescriptorHeap> descHeapRTV;      //RTV用のデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeapMultiRTV; //RTV用のデスクリプタヒープ(マルチ)

public: //メンバ関数
	// レンダーテクスチャの生成
	HRESULT CreateRenderTexture();
	// レンダーターゲットビューの生成
	HRESULT CreateRTV();
	// レンダーターゲットビューの生成(マルチ)
	HRESULT CreateMultiRTV();

	ID3D12Resource* const GetTexBuff(const size_t& index) const { return texBuff[index].Get(); }
	ID3D12DescriptorHeap* const GetSRV() const { return descHeapSRV.Get(); }
	ID3D12DescriptorHeap* const GetRTV() const { return descHeapRTV.Get(); }
	ID3D12DescriptorHeap* const GetMultiRTV() const { return descHeapMultiRTV.Get(); }
};
