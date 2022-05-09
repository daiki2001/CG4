#pragma once
#include <string>
#include <vector>
#include "./Math/EngineMath.h"
#include <DirectXTex.h>

struct Node
{
	std::string name;                    //���O
	EngineMath::Vector3 position;        //���[�J�����W
	EngineMath::Matrix4 rotation;        //���[�J����]�s��
	EngineMath::Vector3 scaling;         //���[�J���X�P�[��
	EngineMath::Matrix4 transform;       //���[�J���ϊ��s��
	EngineMath::Matrix4 globalTransform; //�O���[�o���ϊ��s��
	Node* parent;                        //�e�m�[�h

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
		EngineMath::Vector3 pos;    //xyz���W
		EngineMath::Vector3 normal; //�@���x�N�g��
		DirectX::XMFLOAT2 uv;       //uv���W
	};

private: // �G�C���A�X
	template<class T> using vector = std::vector<T>;

public: // �����o�֐�
	Model();

private: // �����o�ϐ�
	std::string name;      //���f����
	std::string directory; //���f��������f�B���N�g���p�X
	vector<Node> nodes;    //�m�[�h�z��

	Node* meshNode;                     //���b�V�������m�[�h
	vector<VertexPosNormalUv> vertices; //���_�f�[�^�z��
	vector<unsigned short> indices;     //���_�C���f�b�N�X�z��

	EngineMath::Vector3 ambient;      //�A���r�G���g�W��
	EngineMath::Vector3 diffuse;      //�f�B�t���[�Y�W��
	DirectX::TexMetadata metadata;    //�e�N�X�`�����^�f�[�^
	DirectX::ScratchImage scratchImg; //�X�N���b�`�C���[�W
};
