#pragma once
#include "LoadTex.h"
#include "./Math/EngineMath.h"

class PostEffect : public LoadTex
{
private: //エイリアス
	using Vector3 = Engine::Math::Vector3;
	using Matrix4 = Engine::Math::Matrix4;
	
private: //定数
	static const float shaderClearColor[4];
	
public: //メンバ変数
	Vector3 position;        //座標
	float angle;             //回転角度
	Matrix4 matWorld;        //ワールド行列
	DirectX::XMFLOAT4 color; //色
private:
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV用のデスクリプタヒープ
	ComPtr<ID3D12Resource> depthBuff;         //深度バッファ
	ComPtr<ID3D12DescriptorHeap> descHeapRTV; //RTV用のデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeapDSV; //DSV用のデスクリプタヒープ

	int graphHandle;
	size_t spriteCount;

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
};
