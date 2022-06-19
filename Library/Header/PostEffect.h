#pragma once
#include "LoadTex.h"
#include "./Math/EngineMath.h"

class PostEffect : public LoadTex
{
private: //�G�C���A�X
	using Vector3 = Engine::Math::Vector3;
	using Matrix4 = Engine::Math::Matrix4;
	
public: //�����o�ϐ�
	Vector3 position;        //���W
	float angle;             //��]�p�x
	Matrix4 matWorld;        //���[���h�s��
	DirectX::XMFLOAT4 color; //�F
private:
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV�p�̃f�X�N���v�^�q�[�v

	int graphHandle;
	size_t spriteCount;

public: //�����o�֐�
	// �R���X�g���N�^
	PostEffect();

	// ����������
	int Init();
	// �`�揈��
	int Draw();
};
