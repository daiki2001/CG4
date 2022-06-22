#pragma once
#include "LoadTex.h"
#include "./Math/EngineMath.h"

class PostEffect : public LoadTex
{
private: //�G�C���A�X
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using Vector3 = Engine::Math::Vector3;
	using Matrix4 = Engine::Math::Matrix4;
	
private: //�萔
	static const float clearColor[4];
	
public: //�����o�ϐ�
	Vector3 position;        //���W
	float angle;             //��]�p�x
	Matrix4 matWorld;        //���[���h�s��
	DirectX::XMFLOAT4 color; //�F
private:
	ComPtr<ID3D12Resource> vertBuff;          //���_�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW vbView;          //���_�o�b�t�@�r���[
	ComPtr<ID3D12Resource> constBuff;         //�萔�o�b�t�@
	ComPtr<ID3D12Resource> texBuff;           //�e�N�X�`���o�b�t�@
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV�p�̃f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> descHeapRTV; //RTV�p�̃f�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource> depthBuff;         //�[�x�o�b�t�@
	ComPtr<ID3D12DescriptorHeap> descHeapDSV; //DSV�p�̃f�X�N���v�^�q�[�v

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
private:
	// ���_�o�b�t�@�̐���
	HRESULT CreateVertexBuffer();
	// �萔�o�b�t�@�̐���
	HRESULT CreateConstantBuffer();
	// �����_�[�e�N�X�`���̐���
	HRESULT CreateRenderTextrue();
};
