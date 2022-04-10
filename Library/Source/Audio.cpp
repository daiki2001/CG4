#include "./Header/Audio.h"
#include <fstream>
#include <cassert>

#define EoF -1 // End of Function

Audio::Audio() :
	xAudio2(nullptr),
	masterVoice(nullptr),
	soundDatas{}
{
}

Audio::~Audio()
{
	xAudio2.Reset();

	for (size_t i = 0; i < soundDatas.size(); i++)
	{
		SoundUnload(i);
	}
}

HRESULT Audio::Init()
{
	HRESULT hr = S_FALSE;

	// xAudioエンジンのインスタンスを生成
	hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		return hr;
	}

	// マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

int Audio::SoundLoadWave(const char* filePath)
{
	using namespace std;

	// ファイル入力ストリームのインスタンス
	ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filePath, ios_base::binary);
	// ファイルオープン失敗を検出する
	if (file.fail())
	{
		return EoF;
	}

	// RIFFヘッダーの読み込み
	Audio::RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0)
	{
		assert(0);
	}

	// Formatチャンクの読み込み
	Audio::FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(Audio::Chunk));
	// チャンクヘッダーの確認
	if (strncmp(format.chunk.id, "fmt ", 4) != 0)
	{
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	Audio::Chunk data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクかLISTチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0 || strncmp(data.id, "LIST", 4) == 0)
	{
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0)
	{
		assert(0);
	}

	// Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// waveファイルを閉じる
	file.close();

	// returnする為の音声データ
	soundDatas.push_back({});

	soundDatas[soundDatas.size() - 1].wfex = format.fmt;
	soundDatas[soundDatas.size() - 1].pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundDatas[soundDatas.size() - 1].bufferSize = data.size;

	return soundDatas.size() - 1;
}

void Audio::SoundPlayWave(const int& soundData)
{
	HRESULT hr;

	// 波形フォーマットを元にSourceVoiceを生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	hr = xAudio2->CreateSourceVoice(&pSourceVoice, &soundDatas[soundData].wfex);
	assert(SUCCEEDED(hr));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundDatas[soundData].pBuffer;
	buf.AudioBytes = soundDatas[soundData].bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	if (FAILED(hr))
	{
		return;
	}
	hr = pSourceVoice->Start();
	if (FAILED(hr))
	{
		return;
	}
}

void Audio::SoundUnload(const int& soundData)
{
	// バッファのメモリを解放
	delete[] soundDatas[soundData].pBuffer;

	soundDatas[soundData].pBuffer = 0;
	soundDatas[soundData].bufferSize = 0;
	soundDatas[soundData].wfex = {};
}
