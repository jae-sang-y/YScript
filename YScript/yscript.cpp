#include <fstream>
#include <Windows.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
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

void sample()
{
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	{
		SetConsoleOutputCP(CP_UTF8);
		std::ifstream script_file("sample.js");
		if (!script_file.is_open())
			throw std::invalid_argument("Script file not exists.");
		std::string script_content((std::istreambuf_iterator<char>(script_file)), (std::istreambuf_iterator<char>()));
		YScript::ScriptEngine sc(script_content);
	}
}

void main()
{
	sample();
	//if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
	//{
	//	OutputDebugString(L"-----------_CrtMemDumpStatistics ---------");
	//	_CrtMemDumpStatistics(&sDiff);
	//	OutputDebugString(L"-----------_CrtMemDumpAllObjectsSince ---------");
	//	_CrtMemDumpAllObjectsSince(&sOld);
	//	OutputDebugString(L"-----------_CrtDumpMemoryLeaks ---------");
	//	_CrtDumpMemoryLeaks();
	//}
}