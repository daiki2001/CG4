#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>

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

public: //メンバ関数
	// レンダーテクスチャの生成
	HRESULT CreateRenderTexture(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>* const texBuff,
								Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>* descHeapSRV, const UINT& texCount = 1);
	// レンダーターゲットビューの生成
	HRESULT CreateRTV(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>* descHeapRTV,
					  const std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> texBuff);
};
