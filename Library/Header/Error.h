#pragma once
#include <windows.h>
#include <string>
#include <cassert>

#define EF (-1) //Error Function

// �G���[���O���o�̓E�B���h�E�ɏo�͂��A�I������
static void Error(const std::string& str, const char* file, const int& line, const bool& flag = true)
{
	if (flag == false)
	{
		return;
	}

	char logMessage[1024];

	sprintf_s(logMessage, "%s(line:%d)\n", file, line);

	OutputDebugStringA(str.c_str());
	OutputDebugStringA(logMessage);
	assert(0);
	exit(1);

	return;
}

#define ErrorLog(str, flag) Error(str, __FILE__, __LINE__, flag)