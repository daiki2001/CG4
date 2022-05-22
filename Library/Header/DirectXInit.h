#pragma once
#include "Win32API.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <wrl.h>
#include <d3dx12.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

class DirectXInit final : public Win32API
{
private: //シングルトン化
	// コンストラクタをprivateにする
	DirectXInit();
	// コピーコンストラクタを無効にする
	DirectXInit(const DirectXInit& obj) = delete;
	// デストラクタをprivateにする
	~DirectXInit() {}
	// 代入演算子を無効にする
	DirectXInit operator=(const DirectXInit& obj) = delete;
public:
	// インスタンスの取得
	static DirectXInit* GetInstance();

protected: // エイリアス
	// std::を省略
	template<class T> using vector = std::vector<T>;
	// Microsoft::WRL::を省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public: // 静的メンバ関数
	// D3D12デバイスの取得
	static ID3D12Device* GetDevice() { return dev.Get(); };
	// コマンドリストの取得
	static ID3D12GraphicsCommandList* GetCommandList() { return cmdList.Get(); };

private: // 静的メンバ変数
	static ComPtr<ID3D12Device> dev; //D3D12デバイス
	static ComPtr<ID3D12GraphicsCommandList> cmdList; //コマンドリスト

public: // メンバ関数
	// 初期化
	HRESULT Init();

	// 描画前処理
	void ClearScreen();
	// 描画後処理
	void ScreenFlip();

	// ウィンドウサイズの設定
	int SetWindowSize(int width, int height);
private:
	// デバッグレイヤー
	void DebugLayer();

	// 深度バッファの生成
	HRESULT CreateDepthBuffer();

public: // メンバ変数
	float clearColor[4]; //背景色
protected:
	ComPtr<IDXGIFactory6> dxgiFactory;
	ComPtr<IDXGISwapChain4> swapchain;
	ComPtr<ID3D12CommandAllocator> cmdAllocator;
	ComPtr<ID3D12CommandQueue> cmdQueue;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps;

	vector<ComPtr<IDXGIAdapter1>> adapters; //アダプターの列挙用
	ComPtr<IDXGIAdapter1> tmpAdapter;       //ここに特定の名前を持つアダプターオブジェクトが入る

	D3D_FEATURE_LEVEL levels[4]; //対応レベルの配列
	D3D_FEATURE_LEVEL featureLevel;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc;     //標準設定でコマンドキューを生成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc;       //各種設定をしてスワップチェインを生成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;       //各種設定をしてデスクリプタヒープを生成
	vector<ComPtr<ID3D12Resource>> backBuffer; //裏表の2つについて

	// フェンスの生成
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal;
private:
	CD3DX12_RESOURCE_DESC depthResDesc;     //リソース設定
	ComPtr<ID3D12Resource> depthBuffer;     //リソース生成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc; //深度ビュー用設定構造体
	ComPtr<ID3D12DescriptorHeap> dsvHeap;   //深度ビュー用デスクリプタヒープ作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;  //深度ビュー作成

	SIZE_T bbIndex; //バックバッファのインデックス

};
