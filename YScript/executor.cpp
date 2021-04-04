#include "yscript.hpp"

//#define DEBUG_EXECUTER
using namespace YScript;

Executor::Executor(GlobalBinding* const  global, const std::string& namespace_name, const Code& bytecodes)
{
	this->global = global;
	this->local_value_map = &global->local_value_map[namespace_name];

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
			global->local_value_map[namespace_name][argument.size()][argument] = stack.top();
			stack.pop();
		}
		else if (command == "PUSH_VALUE")
		{
		find_from_local:;
			if (auto itr = local_value_map->find(argument.size()); itr == local_value_map->end())
				goto find_from_global;
			if (auto page = (*local_value_map)[argument.size()]; page.find(argument) == page.end())
				goto find_from_global;
			stack.push((*local_value_map)[argument.size()][argument]);
			goto end;
		find_from_global:;
			if (auto itr = global->global_value_map.find(argument.size()); itr == global->global_value_map.end())
				throw std::exception(("Undefined Variable " + argument).c_str());
			if (auto page = global->global_value_map[argument.size()]; page.find(argument) == page.end())
				throw std::exception(("Undefined Variable " + argument).c_str());

			stack.push(global->global_value_map[argument.size()][argument]);
		end:;
		}
		else if (command == "BUILD_DICT")
		{
			stack.push(global->CreateDict());
		}
		else if (command == "BUILD_LIST")
		{
			auto list = List();
			std::stack<YPtr> reverse_stack;
			for (size_t k = 0; k < std::stoull(argument); ++k)
			{
				reverse_stack.push(stack.top());
				stack.pop();
			}
			while (!reverse_stack.empty())
			{
				list.push_back(reverse_stack.top());
				reverse_stack.pop();
			}

			stack.push(global->CreateList(List{ list }));
		}
		else if (command == "IF_FALSE_JUMP")
		{
			auto value = stack.top();
			bool jump = false;

			if (value->CompareType("bool"))
				jump = !value->AsBool();
			else if (value->CompareType("null"))
				jump = true;

			if (jump)
				k = std::stoull(argument) - 1;
			stack.pop();
			while (!stack.empty()) stack.pop();
		}
		else if (command == "JUMP_TO")
		{
			k = std::stoull(argument) - 1;
		}
		else if (command == "OPERATOR")
		{
			if (argument == "INSERT_PAIR")
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
			else if (auto itr = __operator_to_method__.find(argument); itr != __operator_to_method__.end()) {
				auto rhs = stack.top();
				stack.pop();

				auto lhs = stack.top();
				stack.pop();

				auto args = List();
				auto kwargs = Attributes();

				args.push_back(lhs);
				args.push_back(rhs);

				stack.push(global->RunOperator(String(itr->second), lhs, rhs));
			}
			else throw std::exception(("Unknown Operator " + argument).c_str());
		}
		else if (command == "CALL" || command == "CALL_METHOD")
		{
			std::string buf = argument;
			std::list<String> keywords;
			for (size_t pos = buf.find(';'); pos != -1; pos = buf.find(';'))
			{
				keywords.push_back(buf.substr(0, pos));
				buf = buf.substr(pos + 1);
			}

			uint64_t argc = std::stoull(*keywords.begin());
			keywords.pop_front();
			argc -= keywords.size();

			String method_name = "unknown";
			if (command == "CALL_METHOD")
			{
				method_name = *keywords.begin();
				keywords.pop_front();
			}
			auto args = List();
			auto kwargs = Attributes();

			for (auto keyword : keywords)
			{
				kwargs.push_back(Attribute{ keyword, stack.top() });
				stack.pop();
			}
			std::list<YPtr> buffer;
			for (; argc > 1; --argc)
			{
				buffer.push_front(stack.top());
				stack.pop();
			}

			YPtr func = nullptr;
			if (command == "CALL_METHOD")
			{
				YPtr self = stack.top(); stack.pop();
				args.push_back(self);
				func = self->get_attr(method_name);
			}
			else func = stack.top(); stack.pop();

			for (auto arg : buffer)
			{
				args.push_back(arg);
			}
			stack.push(global->CallFunction(func, args, kwargs));
		}
		else if (command == "CLEAR_STACK")
		{
			while (!stack.empty()) stack.pop();
		}
		else if (command == "GET_ATTR")
		{
			auto value = stack.top();
			stack.pop();
			stack.push(value->get_attr(argument));
		}
		else if (command == "GET_SUBSRPT")
		{
			std::deque<YPtr> indexes;
			YPtr obj = global->const_null;
			uint64_t dimention = std::stoull(argument) - 1;
			for (uint64_t k = dimention;; --k)
			{
				if (k > 0)
				{
					indexes.push_front(stack.top());
					stack.pop();
				}
				else
				{
					obj = stack.top();
					stack.pop();
					break;
				}
			}

			auto args_obj = global->CreateList();
			List& args = args_obj->AsList();
			auto kwargs = Attributes();

			for (YPtr index : indexes)
				args.push_back(index);

			stack.push(global->RunOperator("__getitem__", obj, args_obj));
		}
		else throw std::exception("Unknown command");

#ifdef DEBUG_EXECUTER_STACK
		std::stack<YPtr> debug_stack;
		List list = List();
		while (!stack.empty())
		{
			debug_stack.push(stack.top());
			stack.pop();
		}
		while (!debug_stack.empty())
		{
			list.push_back(debug_stack.top());
			stack.push(debug_stack.top());
			debug_stack.pop();
		}
		std::cout << ">>>>>>>> ";
		global->print(global, global->new_list(list), global->const_null);
#endif
	}

	if (!stack.empty())
		throw std::exception("Stack isn't empty");
	}