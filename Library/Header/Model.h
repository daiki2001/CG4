#pragma once
#include <string>
#include <vector>
#include "./Math/EngineMath.h"
#include <d3d12.h>
#include <wrl.h>
#include <DirectXTex.h>

struct Node
{
	std::string name;                      //���O
	Engine::Math::Vector3 position;        //���[�J�����W
	Engine::Math::Matrix4 rotation;        //���[�J����]�s��
	Engine::Math::Vector3 scaling;         //���[�J���X�P�[��
	Engine::Math::Matrix4 transform;       //���[�J���ϊ��s��
	Engine::Math::Matrix4 globalTransform; //�O���[�o���ϊ��s��
	Node* parent;                          //�e�m�[�h

	Node();
};

class Model
{
public: // �t�����h�N���X
	friend class FbxLoader;

public: // �T�u�N���X
	// ���_�f�[�^�\����
	struct VertexPosNormalUv
	{
		Engine::Math::Vector3 pos;    //xyz���W
		Engine::Math::Vector3 normal; //�@���x�N�g��
		DirectX::XMFLOAT2 uv;         //uv���W
	};

private: // �G�C���A�X
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	template<class T> using vector = std::vector<T>;

private: // �ÓI�����o�ϐ�
	static ID3D12Device* dev;

public: // �����o�֐�
	Model();

	void Init();
	void Draw();

	// �e��o�b�t�@�̐���
	int CreateBuffers();

	// �e��o�b�t�@�̐���
	const Engine::Math::Matrix4& GetModelTransform() { return meshNode->globalTransform; }

private: // �����o�ϐ�
	std::string name;      //���f����
	std::string directory; //���f��������f�B���N�g���p�X
	vector<Node> nodes;    //�m�[�h�z��

	Node* meshNode;                     //���b�V�������m�[�h
	vector<VertexPosNormalUv> vertices; //���_�f�[�^�z��
	vector<unsigned short> indices;     //���_�C���f�b�N�X�z��

	ComPtr<ID3D12Resource> vertBuff;          //���_�o�b�t�@
	ComPtr<ID3D12Resource> indexBuff;         //�C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12Resource> texBuff;           //�e�N�X�`���o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW vbView;          //���_�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView;           //�C���f�b�N�X�o�b�t�@�r���[
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV�p�f�X�N���v�^�q�[�v

	Engine::Math::Vector3 ambient;    //�A���r�G���g�W��
	Engine::Math::Vector3 diffuse;    //�f�B�t���[�Y�W��
	DirectX::TexMetadata metadata;    //�e�N�X�`�����^�f�[�^
	DirectX::ScratchImage scratchImg; //�X�N���b�`�C���[�W
};
