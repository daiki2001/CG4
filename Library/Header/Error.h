#pragma once
#include <windows.h>
#include <string>

#define EF (-1) //Error Function

// エラーログを出力ウィンドウに出力し、終了する
static void Error(const std::string& str, const char* file, const int& line)
{
	char logMessage[1024];

	sprintf_s(logMessage, "%s(line:%d)\n", file, line);

	OutputDebugStringA(str.c_str());
	OutputDebugStringA(logMessage);
	exit(1);

	return;
}

#define ErrorLog(str) Error(str, __FILE__, __LINE__)