#pragma once
#include "./Math/EngineMath.h"
#include "./Math/Collision/CollisionInfo.h"
#include <vector>
#include <wrl.h>
#include <d3dx12.h>

/*シェーダ用*/
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

/*DirectDrawingのバージョン指定*/
#define DIRECTDRAWING_VERSION 0x0001

/*ブレンドモード*/
#define BLENDMODE_NOBLEND (0) //ノーブレンド
#define BLENDMODE_ALPHA   (1) //αブレンド
#define BLENDMODE_ADD     (2) //加算合成
#define BLENDMODE_SUB     (3) //減算合成
#define BLENDMODE_INV     (4) //色反転合成

class BaseCollider;

// 頂点データ構造体
struct Vertex
{
	DirectX::XMFLOAT3 pos;    //xyz座標
	DirectX::XMFLOAT3 normal; //法線ベクトル
	DirectX::XMFLOAT2 uv;     //uv座標
};

// 1オブジェクトの頂点情報をまとめた構造体
struct VertexData
{
	/*配列にして使うことが前提*/

private: // エイリアス
	// Microsoft::WRL::を省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	std::vector<Vertex> vertices;    //頂点データ
	ComPtr<ID3D12Resource> vertBuff; //頂点バッファの生成
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; //頂点バッファビューの作成

	std::vector<unsigned short> indices; //インデックスデータ
	ComPtr<ID3D12Resource> indexBuff;    //インデックスバッファの設定
	D3D12_INDEX_BUFFER_VIEW ibView = {};      //インデックスバッファビューの作成
};

// 頂点データ構造体(スプライト用)
struct SpriteVertex
{
	DirectX::XMFLOAT3 pos; //xyz座標
	DirectX::XMFLOAT2 uv;  //uv座標
};

// 定数バッファ用データ構造体
struct ConstBufferData
{
	DirectX::XMFLOAT4 color;    //色 (RGBA)
	DirectX::XMMATRIX mat;      //3D変換行列
	DirectX::XMFLOAT3 lightVec; //光源へのベクトル
};
// 定数バッファ用データ構造体(マテリアル)
struct MaterialConstBufferData
{
	DirectX::XMFLOAT3 ambient;  //アンビエント係数
	float pad1;                 //パディング
	DirectX::XMFLOAT3 diffuse;  //ディフューズ係数
	float pad2;                 //パディング
	DirectX::XMFLOAT3 specular; //スペキュラー係数
	float alpha;                //アルファ
};
// 定数バッファ用データ構造体(スプライト用)
struct SpriteConstBufferData
{
	DirectX::XMFLOAT4 color; //色 (RGBA)
	DirectX::XMMATRIX mat;   //3D変換行列
};

// ディスクリプタハンドル
struct DescHandle
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandle; //CPU側のハンドル
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandle; //GPU側のハンドル
};

// 各オブジェクト(スプライト)で共通のデータをまとめた構造体
struct CommonData
{
	/*'matProjection'の引数*/
	enum Projection
	{
		ORTHOGRAPHIC, //平行投影
		PERSPECTIVE   //透視投影
	};

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelinestate[5]; //パイプラインの状態
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootsignature;    //ルートシグネチャ

	DirectX::XMMATRIX matProjection[2]; //射影行列

	// コンストラクタ
	CommonData();
};

// オブジェクトのワールド行列をまとめた構造体
class Object
{
	/*配列にして使うことが前提*/
public: // メンバ関数
	Object();
	virtual ~Object();

	// 初期化
	virtual void Init();
	// 更新
	virtual void Update();

	// コライダーの設定
	void SetCollider(BaseCollider* collider);
	// 衝突時コールバック関数
	virtual void OnCollision(const CollisionInfo& info) {}

public: // メンバ変数
	int polygonData; //頂点情報の要素番号
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff; //定数バッファ

	DirectX::XMFLOAT3 position; //座標
	DirectX::XMMATRIX rotation; //回転行列
	DirectX::XMFLOAT3 scale;    //スケール
	DirectX::XMFLOAT4 color;    //色

	DirectX::XMMATRIX matWorld; //ワールド行列

	Object* parent; //親の要素番号

	const char* name = nullptr; //クラス名(デバッグ用)
	BaseCollider* collider = nullptr; //コライダー
};

// マテリアル
struct Material
{
	std::string name;            //マテリアル名
	DirectX::XMFLOAT3 ambient;   //アンビエント影響度
	DirectX::XMFLOAT3 diffuse;   //ディフューズ影響度
	DirectX::XMFLOAT3 specular;  //スペキュラー影響度
	float alpha;                 //アルファ
	std::string textureFilename; //テクスチャファイル名

	size_t textrueIndex; //テクスチャのインデックス

	// コンストラクタ
	Material()
	{
		ambient = { 0.3f,0.3f,0.3f };
		diffuse = { 0.0f,0.0f,0.0f };
		specular = { 0.0f,0.0f,0.0f };
		alpha = 1.0f;

		textrueIndex = 0;
	}
};

// objファイルのデータ
class OBJData : public Object
{
	/*配列にして使うことが前提*/
	/*先にモデル不要のオブジェクトを生成する*/
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> materialConstBuff; //定数バッファ
	Material material;

	// コンストラクタ
	OBJData() : Object()
	{
		material = {};
	}
	// デストラクタ
	virtual ~OBJData() override
	{
		Object::~Object();
	}
};

struct OBJSubset
{
	size_t objStart;
	size_t verticesCount;
};

// スプライト一枚分のデータをまとめた構造体
struct Sprite
{
	/*配列にして使うことが前提*/

	Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff;  //頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vbView;                  //頂点バッファビュー
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff; //定数バッファ

	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity(); //ワールド行列
	DirectX::XMFLOAT4 color = { 1, 1, 1, 1 }; //色 (RGBA)

	DirectX::XMFLOAT3 pos = { 0, 0, 0 }; //座標
	float rotation = 0.0f; //回転角
	DirectX::XMFLOAT2 size = { 0.000000001f, 0.000000001f }; //大きさ

	DirectX::XMFLOAT2 anchorpoint = { 0.5f, 0.5f }; //アンカーポイント

	// 描画範囲
	DirectX::XMFLOAT2 texLeftTop = { 0, 0 };
	DirectX::XMFLOAT2 texSize = { 100, 100 };
};

class DirectDrawing
{
public: // サブクラス
	// オブジェクトデータ（スプライトのデータ）のインデックス
	struct IndexData
	{
		int constant; //定数バッファ
		int textrue;  //テクスチャ
	};

protected: // エイリアス
	// DirectX::を省略
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMMATRIX = DirectX::XMMATRIX;
	// std::を省略
	template<class T> using vector = std::vector<T>;
	// Microsoft::WRL::を省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public: // 静的メンバ変数
	static bool isDepthWriteBan; //trueだったら、デプスに対しての上書き禁止
protected:
	static ID3D12Device* dev;                  //D3D12デバイス
	static ID3D12GraphicsCommandList* cmdList; //コマンドリスト
	static vector<Sprite> sprite;              //スプライトのデータ
	static vector<IndexData> spriteIndex;      //スプライトのデータのインデックス

	static CommonData spriteData; //スプライトのデータで共通のデータ

	static size_t blendMode; //ブレンドモード

public: // メンバ関数
	// コンストラクタ
	DirectDrawing();
	// デストラクタ
	~DirectDrawing();

	/// <summary>
	/// 空の定数バッファの生成
	/// </summary>
	/// <param name="pos"> 座標 </param>
	/// <param name="rota"> 回転行列 </param>
	/// <param name="scale"> スケール </param>
	/// <returns> オブジェクトデータの要素番号 </returns>
	int CreateNullConstant(const XMFLOAT3& pos, const XMMATRIX& rota, const XMFLOAT3& scale);
	/// <summary>
	/// 定数バッファの更新
	/// </summary>
	/// <param name="pos"> 座標 </param>
	/// <param name="rota"> 回転行列 </param>
	/// <param name="scale"> スケール </param>
	/// <param name="objectIndex"> オブジェクトデータの要素番号 </param>
	/// <param name="polygonData"> 頂点情報の要素番号 </param>
	/// <param name="parent"> 親の要素番号 </param>
	void UpdataConstant(const XMFLOAT3& pos, const XMMATRIX& rota, const XMFLOAT3& scale,
						const DirectX::XMFLOAT4& color, const int& objectIndex,
						const int& polygonData = -1, Object* parent = nullptr);
	// 描画の際のブレンドモードをセットする
	int SetDrawBlendMode(int blendMode);
	// カメラのニアクリップ距離とファークリップ距離を設定する
	HRESULT SetNearFar(float nearClip, float farClip);

	inline OBJData* GetObjData(int object)
	{
		if (object < 0 || object >= (int)obj.size()) { return nullptr; }
		return &obj[object];
	}
protected:
	// 初期化
	HRESULT Init();
	// オブジェクトの描画関係の初期化
	HRESULT DrawingInit();
	// オブジェクトの描画関係の初期化
	HRESULT MaterialInit();
	// スプライトの描画関係の初期化
	HRESULT SpriteDrawingInit();
	// 頂点バッファとインデックスバッファの生成
	int CreateVertexAndIndexBuffer();
	// 定数バッファの生成
	HRESULT CreateConstBuffer(int* objIndex);
	// スプライトの生成
	int CreateSprite();
	// ディスクリプタヒープの初期化
	HRESULT BasicDescHeapInit();

	// 描画処理の共通部分
	void BaseDrawGraphics();
	// 描画処理の共通部分(スプライト用)
	void BaseDrawSpriteGraphics();
private:
	// データの消去
	void DataClear();
protected: // メンバ変数
	vector<VertexData> vertices; //頂点データ
	Vertex* vertMap;

	vector<OBJData> obj; //オブジェクトデータ
	vector<IndexData> objIndex; //オブジェクトデータのインデックス
	ComPtr<ID3D12DescriptorHeap> basicDescHeap; //定数バッファ用のデスクリプタヒープ
	vector<DescHandle> srvHandle;
	size_t nullBufferCount; //ワールド行列だけのオブジェクトデータの数

	ComPtr<ID3DBlob> vsBlob;    //頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;    //ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob; //エラーオブジェクト
	ComPtr<ID3DBlob> rootSigBlob;

	CommonData objectData[2];   //オブジェクトデータで共通のデータ
	CommonData materialData[2]; //マテリアルデータで共通のデータ

	vector<OBJSubset> objSubset;
private:
	float nearClip; //ニアクリップ距離
	float farClip;  //ファークリップ距離

	// 関数の初期化フラグ
	bool isDrawingInit = false;
	bool isMaterialInit = false;
	bool isSpriteDrawingInit = false;
	bool isBasicDescHeapInit = false;

};
