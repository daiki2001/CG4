#pragma once
#include <map>

/*ƒV[ƒ“‚ğ‚Ü‚½‚®—p*/

class Parameter final
{
private:
	Parameter() {}
	~Parameter() {}
public:
	Parameter(const Parameter&) = delete;
	Parameter operator=(const Parameter&) = delete;

	static void Set(const char* key, const int& vel);
	static int Get(const char* key);

private:
	static std::map<const char*, int> param;

};
