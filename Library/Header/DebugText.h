#pragma once
#include "LoadTex.h"

class DebugText : public LoadTex
{
public: // 定数
	static const int fontWidth = 8;      //文字の横幅
	static const int fontHeight = 16;    //文字の縦幅
	static const int fontLineCount = 16; //一行あたりの文字数

private: // 静的メンバ変数
	static int fontTex;         //フォント画像
	static UINT64 fontTexWidth; //フォント画像の横幅
	static UINT fontTexHeight;  //フォント画像の縦幅

public: // メンバ関数
	// コンストラクタ
	DebugText(const DirectXInit* w);
	// デストラクタ
	~DebugText();

	/// <summary>
	/// デバッグテキスト
	/// </summary>
	/// <param name="posX, posY"> 文字列を描画する領域の左上の座標 </param>
	/// <param name="fontScale"> 文字の大きさ（倍率） </param>
	/// <param name="color"> 色（RGBA） </param>
	/// <param name="text"> 描画したい書式付き文字列のアドレス </param>
	/// <param name="..."> 書式付き文字列に付随する引数 </param>
	/// <returns> 0で成功 </returns>
	HRESULT DrawString(const float& posX, const float& posY, const float& fontScale,
		const XMFLOAT4& color, const char* text, ...);

	// ループ終わり（デバッグテキスト版）
	void TextLoopEnd();
private:
	// データの消去
	void DataClear();

	// デバッグテキストの初期化
	HRESULT DrawStringInit();

private: // メンバ変数
	vector<int> fontIndex; //定数バッファ
	int charCount;         //文字数

};
