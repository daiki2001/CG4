#pragma once
#include <d3d12.h>
#include <wrl.h>

class RenderTexture final
{
private: //�G�C���A�X
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public: //�V���O���g����
	static RenderTexture* Get();
private:
	RenderTexture();
	RenderTexture(const RenderTexture&) = delete;
	~RenderTexture();
	RenderTexture operator=(const RenderTexture&) = delete;

public: //�ÓI�����o�ϐ�
	static float clearColor[4];

private: //�����o�ϐ�
	ComPtr<ID3D12Resource> texBuff[2];        //�e�N�X�`���o�b�t�@
	ComPtr<ID3D12DescriptorHeap> descHeapSRV; //SRV�p�̃f�X�N���v�^�q�[�v

	ComPtr<ID3D12DescriptorHeap> descHeapRTV;      //RTV�p�̃f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> descHeapMultiRTV; //RTV�p�̃f�X�N���v�^�q�[�v(�}���`)

public: //�����o�֐�
	// �����_�[�e�N�X�`���̐���
	HRESULT CreateRenderTexture();
	// �����_�[�^�[�Q�b�g�r���[�̐���
	HRESULT CreateRTV();
	// �����_�[�^�[�Q�b�g�r���[�̐���(�}���`)
	HRESULT CreateMultiRTV();

	ID3D12Resource* const GetTexBuff(const size_t& index) const { return texBuff[index].Get(); }
	ID3D12DescriptorHeap* const GetSRV() const { return descHeapSRV.Get(); }
	ID3D12DescriptorHeap* const GetRTV() const { return descHeapRTV.Get(); }
	ID3D12DescriptorHeap* const GetMultiRTV() const { return descHeapMultiRTV.Get(); }
};
