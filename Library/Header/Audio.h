#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <vector>

#pragma comment(lib, "xaudio2.lib")

class Audio
{
public: // サブクラス

	// チャンクヘッダ
	struct Chunk
	{
		char id[4]; //チャンク毎のID
		INT32 size; //チャンクサイズ
	};
	// RIFFヘッダチャンク
	struct RiffHeader
	{
		Chunk chunk;  //"RIFF"
		char type[4]; //"WAVE"
	};
	// FMTヘッダチャンク
	struct FormatChunk
	{
		Chunk chunk;      //"fmt"
		WAVEFORMATEX fmt; //波形フォーマット
	};
	// 音声データ
	struct SoundData
	{
		WAVEFORMATEX wfex;       //波形フォーマット
		BYTE* pBuffer;           //バッファの先頭アドレス
		unsigned int bufferSize; //バッファのサイズ
	};

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	std::vector<SoundData> soundDatas;

public:
	Audio();
	~Audio();

	// 初期化
	HRESULT Init();

	// 音声データの読み込み
	int SoundLoadWave(const char* filePath);
	// 音声データの再生
	void SoundPlayWave(const int& soundData);
private:
	// 音声データの開放
	void SoundUnload(const int& soundData);
};
