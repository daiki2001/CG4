#pragma once
#include <string>

/*定数*/
namespace Engine
{
namespace General
{
static const std::string libraryDirectory = "./lib/";             //ライブラリディレクトリ
static const std::string defaultTextureFileName = "white1x1.png"; //デフォルト画像
} //General
} //Engine

/*関数*/
namespace Engine
{
namespace General
{
// ディレクトリを含んだファイルパスからファイル名を抽出する
std::string ExtractDirectory(const std::string& path);
// ディレクトリを含んだファイルパスからファイル名を抽出する
std::string ExtractFileName(const std::string& path);
} //General
} //Engine

using namespace Engine;