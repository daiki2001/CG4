#include "./Header/Model.h"
#include "./Header/DirectXInit.h"
#include "./Header/Error.h"

Node::Node() :
	name{},
	position{ 0.0f, 0.0f, 0.0f },
	rotation{ Engine::Math::Identity() },
	scaling{ 1.0f, 1.0f, 1.0f },
	transform{ Engine::Math::Identity() },
	globalTransform{ Engine::Math::Identity() },
	parent{ nullptr }
{
}

ID3D12Device* Model::dev = DirectXInit::GetDevice();

Model::Model() :
	meshNode(nullptr),
	ambient{ 1.0f, 1.0f, 1.0f },
	diffuse{ 1.0f, 1.0f, 1.0f },
	metadata{},
	scratchImg{}
{
	Init();
}

void Model::Init()
{
	dev = DirectXInit::GetDevice();
}

void Model::Draw()
{
}

int Model::CreateBuffers()
{
	using namespace Engine;

	HRESULT hr = S_FALSE;

#pragma region CreateVertexBuffer
	// ���_�f�[�^�S�̂̃T�C�Y
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUv) * vertices.size());
	// ���_�o�b�t�@�̐���
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //�A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(hr))
	{
		return functionError;
	}

	// ���_�o�b�t�@�ւ̃f�[�^�]��
	VertexPosNormalUv* vertMap = nullptr;
	hr = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(hr))
	{
		std::copy(vertices.begin(), vertices.end(), vertMap);
		// �}�b�v������
		vertBuff->Unmap(0, nullptr);
	}

	// ���_�o�b�t�@�r���[�̍쐬
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);
#pragma endregion //CreateVertexBuffer

#pragma region CreateIndexBuffer
	// ���_�C���f�b�N�X�f�[�^�S�̂̃T�C�Y
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	// �C���f�b�N�X�o�b�t�@�̐���
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //�A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));
	if (FAILED(hr))
	{
		return functionError;
	}

	// �C���f�b�N�X�o�b�t�@�ւ̃f�[�^�]��
	unsigned short* indexMap = nullptr;
	hr = indexBuff->Map(0, nullptr, (void**)&indexMap);
	if (SUCCEEDED(hr))
	{
		std::copy(indices.begin(), indices.end(), indexMap);
		// �}�b�v������
		indexBuff->Unmap(0, nullptr);
	}

	// �C���f�b�N�X�o�b�t�@�r���[�̍쐬
	ibView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
#pragma endregion //CreateIndexBuffer

#pragma region CreateTexBuffer
	// �e�N�X�`���摜�f�[�^
	const DirectX::Image* img = scratchImg.GetImage(0, 0, 0);
	assert(img);

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		static_cast<UINT>(metadata.height),
		static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels)
	);

	// �e�N�X�`���o�b�t�@�̐���
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
								 D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff)
	);
	if (FAILED(hr))
	{
		return functionError;
	}

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	hr = texBuff->WriteToSubresource(
		0,
		nullptr,
		img->pixels,
		static_cast<UINT>(img->rowPitch),
		static_cast<UINT>(img->slicePitch)
	);
#pragma endregion //CreateTexBuffer

#pragma region CreateSRV
	// SRV�p�f�X�N���v�^�q�[�v�𐶐�
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 1;
	hr = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeapSRV));
	if (FAILED(hr))
	{
		return functionError;
	}

	// �V�F�[�_�[���\�[�X�r���[�̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	D3D12_RESOURCE_DESC resDesc = texBuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	dev->CreateShaderResourceView(
		texBuff.Get(),
		&srvDesc,
		descHeapSRV->GetCPUDescriptorHandleForHeapStart()
	);
#pragma endregion //CreateSRV

	return 0;
}
