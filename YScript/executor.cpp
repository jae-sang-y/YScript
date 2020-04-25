#include "yscript.hpp"

using namespace YScript;

#define DEBUG_EXECUTER
//#define DEBUG_EXECUTER_STACK

Executor::Executor(GlobalBinding* const  global, const std::vector<std::string>& bytecodes)
{
	this->global = global;
#ifdef DEBUG_EXECUTER_STACK
	for (uint64_t k = 0; k < bytecodes.size(); ++k)
	{
		const std::string command = bytecodes[k].substr(0, bytecodes[k].find('\t'));
		const std::string argument = bytecodes[k].substr(bytecodes[k].find('\t') + 1);
#ifdef DEBUG_EXECUTER
		std::cout << (1000000 + k) << "  " << command << "\t" << argument << '\n';
#endif
	}
#endif
	std::cout << "Run Script\n";

	for (uint64_t k = 0; k < bytecodes.size(); ++k)
	{
		const std::string command = bytecodes[k].substr(0, bytecodes[k].find('\t'));
		const std::string argument = bytecodes[k].substr(bytecodes[k].find('\t') + 1);
#ifdef DEBUG_EXECUTER
		std::cout << (1000000 + k) << "  " << command << "\t" << argument << '\n';
#endif

		if (command == "PUSH_LITERAL")
		{
			stack.push(global->literal_decode(argument));
		}
		else if (command == "STORE_VALUE")
		{
			{
				char buffer[256];
				sprintf_s(buffer, "STATIC 0x%08X \t%s\n", stack.top()->object_id, argument.c_str());
				OutputDebugStringA(buffer);
			}

			if (auto org = global->value_map[argument.size()][argument]; org != nullptr)
			{
				char buffer[256];
				sprintf_s(buffer, "DYNMIC 0x%08X \t%s\n", org->object_id, argument.c_str());
				OutputDebugStringA(buffer);
			}
			global->value_map[argument.size()][argument] = stack.top();
			stack.pop();
		}
		else if (command == "PUSH_VALUE")
		{
			if (auto itr = global->value_map.find(argument.size()); itr == global->value_map.end())
				throw std::exception(("Undefined Variable " + argument).c_str());
			if (auto page = global->value_map[argument.size()]; page.find(argument) == page.end())
				throw std::exception(("Undefined Variable " + argument).c_str());

			stack.push(global->value_map[argument.size()][argument]);
		}
		else if (command == "BUILD_DICT")
		{
			stack.push(std::make_shared<YObject>(YObjectType::Dict, new Dict()));
		}
		else if (command == "BUILD_LIST")
		{
			auto list = new List();
			auto list_obj = std::make_shared<YObject>(YObjectType::List, list);
			std::stack<YPtr> reverse_stack;
			for (size_t k = 0; k < std::stoull(argument); ++k)
			{
				reverse_stack.push(stack.top());
				stack.pop();
			}
			while (!reverse_stack.empty())
			{
				list->push_back(reverse_stack.top());
				reverse_stack.pop();
			}

			stack.push(list_obj);
		}
		else if (command == "BUILD_SET")
		{
			//stack.push(std::make_shared<YObject>(YObjectType::Set, new Set()));
		}
		else if (command == "BUILD_TUPLE")
		{
			//stack.push(std::make_shared<YObject>(YObjectType::Tuple, new Tuple()));
		}
		else if (command == "IF_FALSE_JUMP")
		{
			if (!*(bool*)global->casting(stack.top(), YObjectType::Bool)->data)
				k = std::stoull(argument) - 1;
			stack.pop();
		}
		else if (command == "JUMP_TO")
		{
			k = std::stoull(argument) - 1;
		}
		else if (command == "OPERATOR")
		{
			if (argument == "+" || argument == "-" || argument == "*" || argument == "/" || argument == "%" || argument == "**" ||
				argument == ">" || argument == "<" || argument == ">=" || argument == "<=" || argument == "==" || argument == "!=")
			{
				auto rhs = stack.top();
				stack.pop();

				auto lhs = stack.top();
				stack.pop();

				stack.push(global->operand(argument, lhs, rhs));
			}
			else if (argument == "INSERT_PAIR")
			{
				auto value = stack.top();
				stack.pop();

				auto key = stack.top();
				stack.pop();

				auto dict = stack.top();
				stack.pop();

				(*(Dict*)dict->data).push_back(std::make_pair(key, value));
				stack.push(dict);
			}
			else throw std::exception(("Unknown Operator " + argument).c_str());
		}
		else if (command == "CALL")
		{
			std::string buf = argument;
			std::deque<String> keywords;
			for (size_t pos = buf.find(';'); pos != -1; pos = buf.find(';'))
			{
				keywords.push_back(buf.substr(0, pos));
				buf = buf.substr(pos + 1);
			}

			uint64_t argc = std::stoull(keywords[0]);
			keywords.pop_front();
			YPtr function = nullptr;
			std::deque<YPtr> args_deq;
			std::deque<YPtr> kwargs_deq;
			for (uint64_t k = 1; k < argc; ++k)
			{
				if (argc + kwargs_deq.size() - k - 2 > 0) args_deq.push_front(stack.top());
				else kwargs_deq.push_front(stack.top());
				stack.pop();
			}

			function = stack.top(); stack.pop();

			if (function->type_id == YObjectType::Built_In_Function)
			{
				if (function->data != nullptr)
				{
					auto list = new List();
					auto dict = new Dict();
					auto list_obj = std::make_shared<YObject>(YObjectType::List, list);
					auto dict_obj = std::make_shared<YObject>(YObjectType::Dict, dict);

					for (auto arg : args_deq)
						list->push_back(arg);

					size_t k = 0;
					for (auto kwarg : kwargs_deq)
					{
						dict->push_back(std::pair(std::make_shared<YObject>(YObjectType::String, new String(keywords[k])), kwarg));
						++k;
					}

					stack.push((*(BuiltInFunction*)function->data)(global, list_obj, dict_obj));
				}
				else throw std::exception("Unknown Built in Function ");
			}
			else throw std::exception(("This type is not callable, `" + YObjectTypenames[function->type_id] + "`").c_str());
		}
		else if (command == "CLEAR_STACK")
		{
			while (!stack.empty()) stack.pop();
		}
		else if (command == "LOAD_ATTR")
		{
			auto value = stack.top(); stack.pop();

			YPtr data = nullptr;
			for (auto& pair : value->attrs)
			{
				if (pair.first == argument)
				{
					data = pair.second;
					break;
				}
			}
			if (data == nullptr) throw std::exception("Unknown Attribute");
			stack.push(data);
		}
		else throw std::exception("Unknown command");
	}

#ifdef DEBUG_EXECUTER_STACK
	std::stack<YObject*> debug_stack;
	while (!stack.empty())
	{
		debug_stack.push(stack.top());
		stack.pop();
	}
	while (!debug_stack.empty())
	{
		std::cout << GetRepr(debug_stack.top()) << ", \t";
		stack.push(debug_stack.top());
		debug_stack.pop();
	}
	std::cout << "\r\n";
#endif

	if (!stack.empty())
		throw std::exception("Stack isn't empty");
}