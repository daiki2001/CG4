#pragma once
#include "LoadTex.h"
#include "./Math/EngineMath.h"

class PostEffect : public LoadTex
{
private: //�G�C���A�X
	using Vector3 = Engine::Math::Vector3;
	using Matrix4 = Engine::Math::Matrix4;
	
private: //�萔
	static const float shaderClearColor[4];
	
public: //�����o�ϐ�
	Vector3 position;        //���W
	float angle;             //��]�p�x
	Matrix4 matWorld;        //���[���h�s��
	DirectX::XMFLOAT4 color; //�F
private:
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV�p�̃f�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource> depthBuff;         //�[�x�o�b�t�@
	ComPtr<ID3D12DescriptorHeap> descHeapRTV; //RTV�p�̃f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> descHeapDSV; //DSV�p�̃f�X�N���v�^�q�[�v

	int graphHandle;
	size_t spriteCount;

public: //�����o�֐�
	// �R���X�g���N�^
	PostEffect();

	// ����������
	int Init();
	// �`�揈��
	int Draw();

	// �`��O����
	int PreDraw();
	// �`��㏈��
	int PostDraw();
};
