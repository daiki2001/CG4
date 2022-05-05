#pragma once
#include <string>
#include <vector>
#include "./Math/EngineMath.h"

struct Node
{
	std::string name;                    //名前
	EngineMath::Vector3 position;        //ローカル座標
	EngineMath::Matrix4 rotation;        //ローカル回転行列
	EngineMath::Vector3 scaling;         //ローカルスケール
	EngineMath::Matrix4 transform;       //ローカル変換行列
	EngineMath::Matrix4 globalTransform; //グローバル変換行列
	Node* parent;                        //親ノード

	Node();
};

class Model
{
public: // フレンドクラス
	friend class FbxLoader;

private: // メンバ変数
	std::string name;        //モデル名
	std::vector<Node> nodes; //ノード配列
};
