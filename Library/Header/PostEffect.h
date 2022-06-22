#pragma once
#include "LoadTex.h"
#include "./Math/EngineMath.h"

class PostEffect : public LoadTex
{
private: //エイリアス
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using Vector3 = Engine::Math::Vector3;
	using Matrix4 = Engine::Math::Matrix4;
	
private: //定数
	static const float clearColor[4];
	
public: //メンバ変数
	Vector3 position;        //座標
	float angle;             //回転角度
	Matrix4 matWorld;        //ワールド行列
	DirectX::XMFLOAT4 color; //色
private:
	ComPtr<ID3D12Resource> vertBuff;          //頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vbView;          //頂点バッファビュー
	ComPtr<ID3D12Resource> constBuff;         //定数バッファ
	ComPtr<ID3D12Resource> texBuff;           //テクスチャバッファ
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV用のデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeapRTV; //RTV用のデスクリプタヒープ
	ComPtr<ID3D12Resource> depthBuff;         //深度バッファ
	ComPtr<ID3D12DescriptorHeap> descHeapDSV; //DSV用のデスクリプタヒープ

public: //メンバ関数
	// コンストラクタ
	PostEffect();

	// 初期化処理
	int Init();
	// 描画処理
	int Draw();

	// 描画前処理
	int PreDraw();
	// 描画後処理
	int PostDraw();
private:
	// 頂点バッファの生成
	HRESULT CreateVertexBuffer();
	// 定数バッファの生成
	HRESULT CreateConstantBuffer();
	// レンダーテクスチャの生成
	HRESULT CreateRenderTextrue();
};
