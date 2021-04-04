#include <fstream>
#include <sstream>
#include <Windows.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

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
	{
	}
}

int main(int argc, char** argv)
{
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	SetConsoleOutputCP(CP_UTF8);

	if (argc == 2) { // Execute a file
		//std::cout << "argv[0]" << argv[1] << std::endl;
		std::ifstream script_file(argv[1]);
		if (!script_file.is_open())
		{
			std::cout << "Can't open a script file." << std::endl;
			return -1;
		}
		std::string script_content((std::istreambuf_iterator<char>(script_file)), (std::istreambuf_iterator<char>()));
		YScript::ScriptEngine sc(script_content);
	}
	else if (argc <= 1) { // Execute as terminal
		std::cout << "YScript 0.0.1 on Windows" << std::endl;
		std::ostringstream out;
		char buf[4096];
		while (true) {
			int index = 0;
			std::cout << ">>> ";
			memset(buf, 0x00, 4096);
			out.str("");
			out.clear();
			while (true) {
				if (index > 0) std::cout << "... ";

				std::cin.getline(buf, 4096);
				if (strlen(buf) == 0) break;

				if (index++ > 0) out << "\n";
				out << buf;
			};
			try {
				YScript::ScriptEngine sc(out.str());
			}
			catch (std::exception e) {
				std::cout << "Exception: " << e.what() << std::endl;
			}
		}
	}
	else {
		std::cout << "yscript.exe [input file]";
		return -1;
	}
	return 0;
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