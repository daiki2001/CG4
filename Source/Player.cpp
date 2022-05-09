#include "./Header/Player.h"
#include "./Header/Camera.h"
#include "./Header/SafeDelete.h"
#include "./Header/Error.h"

const float Player::sphereRadius = 3.0f;

Player* Player::Get()
{
	static Player instance;
	return &instance;
}

Player::Player() :
	OBJData(),
	draw(nullptr),
	object(-1),
	isObj(false)
{
}

Player::~Player()
{
	safe_delete(draw);

	OBJData::~OBJData();
}

void Player::Init()
{
	position = Camera::target;
}

void Player::Update()
{

}

void Player::Draw()
{
	static OBJData* objData = nullptr;
	objData = draw->GetObjData(object);

	if (isObj)
	{
		draw->DrawOBJ(object, objData->position, objData->rotation, objData->scale);
	}
	else
	{
		draw->Draw(object, objData->position, objData->rotation, objData->scale, objData->color);
	}
}

int Player::CreateModel(const char* filePath, const char* directoryPath)
{
	if (object != -1)
	{
		return 0;
	}

	if (draw == nullptr)
	{
		ErrorLog("描画用オブジェクトがありません\n", true);
		return EF;
	}

	if (filePath == nullptr || directoryPath == nullptr)
	{
		object = draw->CreateSphere(sphereRadius, 16);
		isObj = false;
	}
	else
	{
		object = draw->CreateOBJModel(filePath, directoryPath);
		isObj = true;
	}

	return 0;
}

void Player::OnCollision(const CollisionInfo& info)
{
	draw->DrawString(0.0f, 0.0f, 1.0f, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), "Hit");
}
