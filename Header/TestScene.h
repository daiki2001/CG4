#pragma once
#include "./Header/BaseScene.h"
#include "./Math/EngineMath.h"
#include "./Math/Collision/Collision.h"
#include "Player.h"
#include "./Header/FbxLoader.h"
#include "./Header/Model.h"
#include "./Header/PostEffect.h"

#include <memory>

class TestScene : public BaseScene
{
public: // �G�C���A�X
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	using Vector3 = Engine::Math::Vector3;

public: // �萔

public: // �����o�֐�
	TestScene(SceneChenger* sceneChenger);
	~TestScene() override;

	void Init() override;
	void Update() override;
	void Draw() override;

private: // �����o�ϐ�
	int background; //�w�i�摜

	PostEffect postEffect;
	FbxLoader* fbxLoader;

	// �I�u�W�F�N�g�̐���
	int fbxModel; //FBX���f��
};
