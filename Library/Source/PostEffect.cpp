#include "./Header/PostEffect.h"
#include "./Header/DirectXInit.h"
#include "./Header/Error.h"

PostEffect::PostEffect() :
	LoadTex(),
	position(0.0f, 0.0f, 0.0f),
	angle(0.0f),
	matWorld(Engine::Math::Identity()),
	color(1.0f, 1.0f, 1.0f, 1.0f),
	graphHandle(-1),
	spriteCount(size_t(-1))
{
}

int PostEffect::Init()
{
	graphHandle = LoadTextrue(L"./lib/white1x1.png");

	return graphHandle;
}

int PostEffect::Draw()
{
	using namespace DirectX;
	using namespace Engine;

	static auto* cmdList = DirectXInit::GetCommandList();
	static bool isInit = false;

	if (isInit == false)
	{
		int size = DrawTextureInit();
		if (size == FUNCTION_ERROR)
		{
			return FUNCTION_ERROR;
		}

		spriteIndex.push_back({ size, graphHandle });
		spriteCount = spriteIndex.size() - 1;

		enum Corner { LB, LT, RB, RT };

		SpriteVertex vert[] = {
			{{}, { 0.0f, 1.0f }},
			{{}, { 0.0f, 0.0f }},
			{{}, { 1.0f, 1.0f }},
			{{}, { 1.0f, 0.0f }},
		};

		float left = 0.0f;
		float right = 100.0f;
		float top = 0.0f;
		float bottom = 100.0f;

		vert[LB].pos = { left, bottom, 0.0f };
		vert[LT].pos = { left, top, 0.0f };
		vert[RB].pos = { right, bottom, 0.0f };
		vert[RT].pos = { right, top, 0.0f };

		// 頂点バッファへのデータ転送
		SpriteVertex* vertexMap = nullptr;
		sprite[spriteIndex[spriteCount].constant].vertBuff->Map(0, nullptr, (void**)&vertexMap);
		memcpy(vertexMap, vert, sizeof(vert));
		sprite[spriteIndex[spriteCount].constant].vertBuff->Unmap(0, nullptr);

		isInit = true;
	}

	if (spriteIndex.size() == 0)
	{
		return FUNCTION_ERROR;
	}

#pragma region GraphicsCommand

	BaseDrawSpriteGraphics();

	IndexData& index = spriteIndex[spriteCount];

	matWorld = Math::Identity();
	matWorld *=Math::rotateZ(angle * Math::degree);
	matWorld *= Math::translate(Vector3(
		sprite[index.constant].pos.x,
		sprite[index.constant].pos.y,
		0.0f
	));

	//if (parent != -1)
	//{
	//	sprite[index.constant].matWorld *= sprite[parent].matWorld;
	//}

	SpriteConstBufferData* constMap = nullptr;
	HRESULT hr = sprite[index.constant].constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(hr))
	{
		constMap->color = color;
		constMap->mat = matWorld * spriteData.matProjection[CommonData::Projection::ORTHOGRAPHIC];
		sprite[index.constant].constBuff->Unmap(0, nullptr);
	}

	// デスクリプタヒープをセット
	ID3D12DescriptorHeap* ppHeaps[] = { texCommonData.descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット
	cmdList->SetGraphicsRootConstantBufferView(0, sprite[index.constant].constBuff->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(1, textrueData[index.textrue].gpuDescHandle);

	// 頂点バッファの設定
	cmdList->IASetVertexBuffers(0, 1, &sprite[index.constant].vbView);
	// 描画コマンド
	cmdList->DrawInstanced(4, 1, 0, 0);

#pragma endregion

	return index.constant;
}
