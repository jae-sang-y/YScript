#include <fstream>
#include <Windows.h>

#define DEBUG_TREE

#include "yscript.hpp"

namespace YScript
{
	bool operator == (const Token& left, const Token& right)
	{
		return left.type == right.type && left.str == right.str;
	}

	bool operator != (const Token& left, const Token& right)
	{
		return !(left == right);
	}
};
void main()
{
	SetConsoleOutputCP(CP_UTF8);
	std::ifstream script_file("sample.js");
	if (!script_file.is_open())
		throw std::invalid_argument("Script file not exists.");
	std::string script_content((std::istreambuf_iterator<char>(script_file)), (std::istreambuf_iterator<char>()));
	YScript::ScriptEngine sc(script_content);
}