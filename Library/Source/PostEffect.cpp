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
	using namespace Engine;

	HRESULT hr = S_FALSE;

	int size = CreateSprite();
	if (size == FUNCTION_ERROR)
	{
		return FUNCTION_ERROR;
	}

	spriteIndex.push_back({ size, 0 });
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

	// ���_�o�b�t�@�ւ̃f�[�^�]��
	SpriteVertex* vertexMap = nullptr;
	sprite[spriteIndex[spriteCount].constant].vertBuff->Map(0, nullptr, (void**)&vertexMap);
	memcpy(vertexMap, vert, sizeof(vert));
	sprite[spriteIndex[spriteCount].constant].vertBuff->Unmap(0, nullptr);

	// �e�N�X�`�����\�[�X�ݒ�
	D3D12_RESOURCE_DESC texResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		static_cast<UINT64>(DirectXInit::GetInstance()->windowWidth),
		static_cast<UINT>(DirectXInit::GetInstance()->windowHeight),
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	auto* dev = DirectXInit::GetDevice();

	// �e�N�X�`���o�b�t�@�̐���
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
								 D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texResDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //�e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&textrueData[0].texbuff)
	);
	if (FAILED(hr))
	{
		return FUNCTION_ERROR;
	}

	// �e�N�X�`����ԃN���A
	{
		// ��f��
		const UINT pixleCount = DirectXInit::GetInstance()->windowWidth *
			DirectXInit::GetInstance()->windowHeight;
		// �摜1�s���̃f�[�^�T�C�Y
		const UINT rowPitch = sizeof(UINT) * DirectXInit::GetInstance()->windowWidth;
		// �摜�S�̂̃f�[�^�T�C�Y
		const UINT depthPitch = rowPitch * DirectXInit::GetInstance()->windowHeight;
		// �摜�C���[�W
		UINT* img = new UINT[pixleCount];
		for (UINT i = 0; i < pixleCount; i++)
		{
			img[i] = 0xff0000ff;
		}

		// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
		hr = textrueData[0].texbuff->WriteToSubresource(
			0,
			nullptr,
			img,
			rowPitch,
			depthPitch
		);
		if (FAILED(hr))
		{
			return hr;
		}
		delete[] img;
	}

	// SRV�p�f�X�N���v�^�q�[�v��ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 1;
	// SRV�p�f�X�N���v�^�q�[�v�𐶐�
	hr = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeapSRV));
	if (FAILED(hr))
	{
		return hr;
	}

	// �V�F�[�_�[���\�[�X�r���[�̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textrueData[0].texbuff->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// �f�X�N���v�^�q�[�v��SRV�쐬
	dev->CreateShaderResourceView(
		textrueData[0].texbuff.Get(),
		&srvDesc,
		descHeapSRV->GetCPUDescriptorHandleForHeapStart()
	);

	return 0;
}

int PostEffect::Draw()
{
	using namespace DirectX;
	using namespace Engine;

	static auto* cmdList = DirectXInit::GetCommandList();
	static bool isInit = false;

	if (isInit == false)
	{
		isInit = true;
	}

	if (spriteIndex.size() == 0)
	{
		return FUNCTION_ERROR;
	}

#pragma region GraphicsCommand

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

	// �r���[�|�[�g�̈�̐ݒ�
	cmdList->RSSetViewports(
		1,
		&CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<float>(DirectXInit::GetInstance()->windowWidth),
			static_cast<float>(DirectXInit::GetInstance()->windowHeight)
		)
	);

	// �V�U�[��`�̐ݒ�
	cmdList->RSSetScissorRects(
		1,
		&CD3DX12_RECT(
			0,
			0,
			DirectXInit::GetInstance()->windowWidth,
			DirectXInit::GetInstance()->windowHeight
		)
	);

	cmdList->SetPipelineState(spriteData.pipelinestate[blendMode].Get());
	cmdList->SetGraphicsRootSignature(spriteData.rootsignature.Get());

	// �v���~�e�B�u�`��̐ݒ�
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// �f�X�N���v�^�q�[�v���Z�b�g
	ID3D12DescriptorHeap* ppHeaps[] = { descHeapSRV.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// �萔�o�b�t�@�r���[���Z�b�g
	cmdList->SetGraphicsRootConstantBufferView(0, sprite[index.constant].constBuff->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(1, descHeapSRV->GetGPUDescriptorHandleForHeapStart());

	// ���_�o�b�t�@�̐ݒ�
	cmdList->IASetVertexBuffers(0, 1, &sprite[index.constant].vbView);
	// �`��R�}���h
	cmdList->DrawInstanced(4, 1, 0, 0);

#pragma endregion

	return index.constant;
}
