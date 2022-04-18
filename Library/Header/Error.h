#pragma once
#include <windows.h>
#include <string>
#include <cassert>

#define EF (-1) //Error Function

// エラーログを出力ウィンドウに出力し、終了する
static void Error(const std::string& str, const char* file, const int& line, const bool& flag = true)
{
	if (flag == false)
	{
		return;
	}

	char logMessage[1024];

	sprintf_s(logMessage, "%s(line:%d)", file, line);

	assert(1);
	OutputDebugStringA(str.c_str());
	OutputDebugStringA(logMessage);
	exit(1);

	return;
}

#define ErrorLog(str) Error(str, __FILE__, __LINE__)
//#define ErrorLog(str, flag) Error(str, __FILE__, __LINE__, flag)