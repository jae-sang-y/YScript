#include "yscript.hpp"

using namespace YScript;

//#define DEBUG_EXECUTER
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
			stack.push(literal_decode(argument));
		}
		else if (command == "STORE_VALUE")
		{
			stack.top()->is_volatility = false;

			{
				char buffer[256];
				sprintf_s(buffer, "STATIC 0x%08X \t%s\n", stack.top()->object_id, argument.c_str());
				OutputDebugStringA(buffer);
			}

			if (auto org = global->value_map[argument.size()][argument]; org != nullptr)
			{
				org->is_volatility = true;
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
		else if (command == "IF_FALSE_JUMP")
		{
			auto data = casting(stack.top(), YObjectType::Bool);
			if (!*(bool*)data->data)
				k = std::stoull(argument) - 1;
			delete data;
			if (stack.top()->is_volatility)
				delete stack.top();
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

				stack.push(operand(argument, deepcopy(lhs), deepcopy(rhs)));
				if (lhs->is_volatility) delete lhs;
				if (rhs->is_volatility) delete rhs;
			}
			else throw std::exception(("Unknown Operator " + argument).c_str());
		}
		else if (command == "CALL")
		{
			uint64_t argc = std::stoull(argument);
			YObject* function = nullptr;
			std::vector<YObject*> arguments;
			for (uint64_t k = 1; k < argc; ++k)
			{
				arguments.push_back(stack.top()); stack.pop();
			}
			function = stack.top(); stack.pop();

			uint64_t function_id = *(uint64_t*)function->data;
			if (function_id == 0x01)
			{
				std::cout << print(arguments.data(), arguments.size());
			}
			else throw std::exception(("Unknown Function " + std::to_string(function_id)).c_str());

			for (auto argument : arguments)
			{
				if (argument->is_volatility)
					delete argument;
			}

			if (function->is_volatility)
				delete function;
		}
		else if (command == "CLEAR_STACK")
		{
			while (!stack.empty())
			{
				if (auto& obj = stack.top(); obj->is_volatility)
				{
					std::cout << "delete: " << __repr__(obj) << "\r\n";
					delete stack.top();
				}
				stack.pop();
			}
		}
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