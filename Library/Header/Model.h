#pragma once
#include <string>
#include <vector>
#include "./Math/EngineMath.h"

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

private: // �����o�ϐ�
	std::string name;        //���f����
	std::vector<Node> nodes; //�m�[�h�z��
};
