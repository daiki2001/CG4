#pragma once
#include "DebugText.h"
#include <d3dx12.h>
#include "./Header/Camera.h"

#define scale_xyz(_scale) DirectX::XMFLOAT3(_scale, _scale, _scale)

class DrawPolygon final : public DebugText
{
public: // メンバ関数
	// コンストラクタ
	DrawPolygon();
	// デストラクタ
	~DrawPolygon();

	// 三角形の作成
	int CreateTriangle(const XMFLOAT3& vertex1, const DirectX::XMFLOAT2& uv1,
					   const XMFLOAT3& vertex2, const DirectX::XMFLOAT2& uv2,
					   const XMFLOAT3& vertex3, const DirectX::XMFLOAT2& uv3, const bool& isFill = true);
	// 矩形の作成
	int CreateRect(const float& width, const float& height, const bool& isFill = true);
	// 正多角形の作成
	int CreateCircle(const float& r, const size_t& divNum, const bool& isFill = true);
	// 直方体の作成
	int Create3Dbox(const float& width, const float& height, const float& depth, const bool& isFill = true);
	// 正多角錐の作成
	int CreateCorn(const float& r, const float& h, const size_t& divNum, const bool& isFill = true);
	// 正多角柱の作成
	int CreateCylinder(const float& r, const float& h, const size_t& divNum, const bool& isFill = true);
	// 球体の作成
	int CreateSphere(const float& r, const size_t& divNum, const bool& isFill = true);

	// objファイルによるモデルの作成（filePath：objファイルの名前、directoryPath：モデルのファイルがあるフォルダ）
	int CreateOBJModel(const char* filePath, const char* directoryPath);

	// テクスチャのタイリング
	void Tiring(const int& polygonData, const float& uvScaleX, const float& uvScaleY);
	// テクスチャの自動タイリング
	void AutoTiring(const int& polygonData, const int& graphHandle);
	// テクスチャの移動
	void MoveUV(const int& polygonData, const int& graphHandle, DirectX::XMFLOAT2 speed);

	// オブジェクトの描画処理
	int Draw(const int& polygonData, const XMFLOAT3& position, const XMMATRIX& rotation, const XMFLOAT3& scale,
			 const XMFLOAT4& color, const int& graphHandle = 0, const bool& isFill = true,
			 const bool& isOrthoGraphic = false, const bool& viewMultiFlag = true, Object* parent = nullptr);

	// モデルの描画処理
	int DrawOBJ(const int& object, const XMFLOAT3& position, const XMMATRIX& rotation, const XMFLOAT3& scale,
				const XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
				const bool& isOrthoGraphic = false, const bool& viewMultiFlag = true, Object* parent = nullptr);

	// カメラの作成
	int CreateCamera(const XMFLOAT3& cameraPos, const XMFLOAT3& cameraTarget, const XMFLOAT3& upVector);

	// スクリーン座標からワールド座標を求める
	DirectX::XMFLOAT3 ScreenToWorld(int x, int y, float z);
	// ビュー行列の逆行列を取得（移動量は0固定）
	DirectX::XMMATRIX InverseView();

	// 内部の初期化用関数
	void PolygonLoopEnd();
private:
	// 描画関数の初期化
	int DrawPolygonInit();

	// 描画関数の初期化
	int DrawOBJInit();

	// マテリアル読み込み
	void LoadMaterial(const std::string& directoryPath, const std::string& filename);

	/*頂点座標とインデックスデータ計算用*/
	void Circle(const XMFLOAT3& centerPos, const float& r, const size_t& divNum,
				const bool& flag, Vertex* v, unsigned short* index);

	// データの消去
	void DataClear();

public: // メンバ変数
	XMFLOAT3 light;    //光源
private:
	int polygonCount; //オブジェクトの数
	size_t loopCount; //ループした回数

	XMFLOAT3 lightVec; //光線

	size_t objModelCount; //objファイルから読み込んだモデルの数
	vector<size_t> verticesCount; //
	vector<Material> material;

	// 関数の初期化フラグ
	bool isDrawOBJPolygonInit = false;
};
