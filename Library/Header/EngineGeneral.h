#pragma once
#include <string>

namespace Engine
{
/*定数*/
namespace General
{
static const std::string libraryDirectory = "./lib/";             //ライブラリディレクトリ
static const std::string defaultTextureFileName = "white1x1.png"; //デフォルト画像
} //General

/*関数*/
namespace General
{
// ディレクトリを含んだファイルパスからファイル名を抽出する
std::string ExtractDirectory(const std::string& path);
// ディレクトリを含んだファイルパスからファイル名を抽出する
std::string ExtractFileName(const std::string& path);
} //General
} //Engine

using namespace Engine;