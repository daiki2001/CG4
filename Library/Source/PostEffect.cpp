#include "./Header/PostEffect.h"
#include "./Header/DirectXInit.h"
#include "./Header/Error.h"

const float PostEffect::clearColor[4] = { 1.0f, 0.5f, 0.0f, 0.0f };

PostEffect::PostEffect() :
	LoadTex(),
	position(0.0f, 0.0f, 0.0f),
	angle(0.0f),
	matWorld(Engine::Math::Identity()),
	color(1.0f, 1.0f, 1.0f, 1.0f),
	vertBuff{},
	vbView{},
	constBuff{},
	texBuff{},
	descHeapSRV{},
	descHeapRTV{},
	depthBuff{},
	descHeapDSV{}
{
}

int PostEffect::Init()
{
	using namespace Engine;

	HRESULT hr = S_FALSE;
	DirectXInit* w = DirectXInit::GetInstance();
	auto* dev = DirectXInit::GetDevice();

	hr = CreateVertexBuffer();
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	// �萔�o�b�t�@�̐���
	hr = CreateConstantBuffer();
	if (FAILED(hr))
	{
		return Engine::FUNCTION_ERROR;
	}

	hr = CreateRenderTextrue();
	if (FAILED(hr))
	{
		return hr;
	}

	// SRV�p�f�X�N���v�^�q�[�v��ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC srvDescHeapDesc = {};
	srvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescHeapDesc.NumDescriptors = 1;
	// SRV�p�f�X�N���v�^�q�[�v�𐶐�
	hr = dev->CreateDescriptorHeap(&srvDescHeapDesc, IID_PPV_ARGS(&descHeapSRV));
	if (FAILED(hr))
	{
		return hr;
	}

	// �V�F�[�_�[���\�[�X�r���[�̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texBuff->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// �f�X�N���v�^�q�[�v��SRV�쐬
	dev->CreateShaderResourceView(
		texBuff.Get(),
		&srvDesc,
		descHeapSRV->GetCPUDescriptorHandleForHeapStart()
	);

	// RTV�p�f�X�N���v�^�q�[�v��ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc = {};
	rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHeapDesc.NumDescriptors = 1;
	// RTV�p�f�X�N���v�^�q�[�v�𐶐�
	hr = dev->CreateDescriptorHeap(&rtvDescHeapDesc, IID_PPV_ARGS(&descHeapRTV));
	if (FAILED(hr))
	{
		return hr;
	}

	// �����_�[�^�[�Q�b�g�r���[�̐���
	dev->CreateRenderTargetView(
		texBuff.Get(),
		nullptr,
		descHeapRTV->GetCPUDescriptorHandleForHeapStart()
	);

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		w->windowWidth,
		w->windowHeight,
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	// ���\�[�X����
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //�[�x�l�������݂Ɏg�p
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
		IID_PPV_ARGS(&depthBuff)
	);
	if (FAILED(hr))
	{
		return hr;
	}

	// DSV�p�f�X�N���v�^�q�[�v�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescHeapDesc = {};
	dsvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescHeapDesc.NumDescriptors = 1;
	// DSV�p�f�X�N���v�^�q�[�v�쐬
	hr = dev->CreateDescriptorHeap(&dsvDescHeapDesc, IID_PPV_ARGS(&descHeapDSV));
	if (FAILED(hr))
	{
		return hr;
	}

	// �f�X�N���v�^�q�[�v��DSV�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; //�[�x�l�t�H�[�}�b�g
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dev->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		descHeapDSV->GetCPUDescriptorHandleForHeapStart()
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

#pragma region GraphicsCommand

	SpriteConstBufferData* constMap = nullptr;
	HRESULT hr = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(hr))
	{
		constMap->color = color;
		constMap->mat = Math::Identity();
		constBuff->Unmap(0, nullptr);
	}

	cmdList->SetPipelineState(spriteData.pipelinestate[blendMode].Get());
	cmdList->SetGraphicsRootSignature(spriteData.rootsignature.Get());

	// �v���~�e�B�u�`��̐ݒ�
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// �f�X�N���v�^�q�[�v���Z�b�g
	ID3D12DescriptorHeap* ppHeaps[] = { descHeapSRV.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// �萔�o�b�t�@�r���[���Z�b�g
	cmdList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(1, descHeapSRV->GetGPUDescriptorHandleForHeapStart());

	// ���_�o�b�t�@�̐ݒ�
	cmdList->IASetVertexBuffers(0, 1, &vbView);
	// �`��R�}���h
	cmdList->DrawInstanced(4, 1, 0, 0);

#pragma endregion

	return hr;
}

int PostEffect::PreDraw()
{
	static auto* cmdList = DirectXInit::GetCommandList();

	DirectXInit* w = DirectXInit::GetInstance();

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		texBuff.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	));

	// �����_�[�^�[�Q�b�g�r���[�p�f�B�X�N���v�^�q�[�v�̃n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE rtvH = descHeapRTV->GetCPUDescriptorHandleForHeapStart();
	// �[�x�X�e���V���r���[�p�f�B�X�N���v�^�q�[�v�̃n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH = descHeapDSV->GetCPUDescriptorHandleForHeapStart();
	// �����_�[�^�[�Q�b�g���Z�b�g
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	// �r���[�|�[�g�̈�̐ݒ�
	cmdList->RSSetViewports(
		1,
		&CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<float>(w->windowWidth),
			static_cast<float>(w->windowHeight)
		)
	);

	// �V�U�[��`�̐ݒ�
	cmdList->RSSetScissorRects(
		1,
		&CD3DX12_RECT(
			0,
			0,
			w->windowWidth,
			w->windowHeight
		)
	);

	// ��ʃN���A
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	return 0;
}

int PostEffect::PostDraw()
{
	static auto* cmdList = DirectXInit::GetCommandList();

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		texBuff.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	));

	return 0;
}

HRESULT PostEffect::CreateVertexBuffer()
{
	HRESULT hr = S_FALSE;
	auto* dev = DirectXInit::GetDevice();

	SpriteVertex vert[] = {
		{{ -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f }},
		{{ -0.5f, +0.5f, 0.0f }, { 0.0f, 0.0f }},
		{{ +0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }},
		{{ +0.5f, +0.5f, 0.0f }, { 1.0f, 0.0f }},
	};

	// ���_�o�b�t�@�̐���
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //�A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vert)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(hr))
	{
		return hr;
	}

	// ���_�o�b�t�@�ւ̃f�[�^�]��
	SpriteVertex* vertexMap = nullptr;
	hr = vertBuff->Map(0, nullptr, (void**)&vertexMap);
	if (SUCCEEDED(hr))
	{
		memcpy(vertexMap, vert, sizeof(vert));
		vertBuff->Unmap(0, nullptr);
	}

	// ���_�o�b�t�@�r���[�̐���
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vert);
	vbView.StrideInBytes = sizeof(SpriteVertex);

	return hr;
}

HRESULT PostEffect::CreateConstantBuffer()
{
	HRESULT hr = S_FALSE;
	auto* dev = DirectXInit::GetDevice();

	// �萔�o�b�t�@�̐���
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //�A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xFF) & ~0xFF), //���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	if (FAILED(hr))
	{
		return hr;
	}

	// �萔�o�b�t�@�Ƀf�[�^�]��
	SpriteConstBufferData* constMap = nullptr;
	hr = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(hr))
	{
		constMap->color = color;
		constMap->mat = Engine::Math::Identity();
		constBuff->Unmap(0, nullptr);
	}

	return hr;
}

HRESULT PostEffect::CreateRenderTextrue()
{
	using namespace Engine;

	HRESULT hr = S_FALSE;
	DirectXInit* w = DirectXInit::GetInstance();
	auto* dev = DirectXInit::GetDevice();

	// �e�N�X�`�����\�[�X�ݒ�
	D3D12_RESOURCE_DESC texResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		static_cast<UINT64>(w->windowWidth),
		static_cast<UINT>(w->windowHeight),
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	// �e�N�X�`���o�b�t�@�̐���
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
								 D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texResDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //�e�N�X�`���p�w��
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clearColor),
		IID_PPV_ARGS(&texBuff)
	);
	if (FAILED(hr))
	{
		return hr;
	}

	// �e�N�X�`����ԃN���A
	//{
	//	// ��f��
	//	const UINT pixleCount = w->windowWidth * w->windowHeight;
	//	// �摜1�s���̃f�[�^�T�C�Y
	//	const UINT rowPitch = sizeof(UINT) * w->windowWidth;
	//	// �摜�S�̂̃f�[�^�T�C�Y
	//	const UINT depthPitch = rowPitch * w->windowHeight;
	//	// �摜�C���[�W
	//	UINT* img = new UINT[pixleCount];
	//	for (UINT i = 0; i < pixleCount; i++)
	//	{
	//		img[i] = 0xff0000ff;
	//	}

	//	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	//	hr = texBuff->WriteToSubresource(
	//		0,
	//		nullptr,
	//		img,
	//		rowPitch,
	//		depthPitch
	//	);
	//	if (FAILED(hr))
	//	{
	//		return hr;
	//	}
	//	delete[] img;
	//}

	return hr;
}
