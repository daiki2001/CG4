#include "../Header/Parameter.h"

std::map<const char*, int> Parameter::param = {};

void Parameter::Set(const char* key, const int& vel)
{
	param[key] = vel;
}

int Parameter::Get(const char* key)
{
	auto itr = param.find(key);

	if (itr == param.end())
	{
		return -1;
	}

	return itr->second;
}
