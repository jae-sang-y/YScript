#include "yscript.hpp"

using namespace YScript;

//#define DEBUG_EXECUTER
//#define DEBUG_EXECUTER_STACK

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
			stack.push(global->new_dict(Dict()));
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

			stack.push(global->new_list(list));
		}
		else if (command == "IF_FALSE_JUMP")
		{
			auto value = stack.top();
			auto obj = global->call_built_in_function(global->type_bool->get_attr("__new__")->data, global->const_null, global->const_null);

			global->call_built_in_method(obj->get_attr("__init__")->data, obj,
				global->new_list(List{ value }), global->const_null);
			if (!*(bool*)obj->data)
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
				stack.push(global->call_built_in_method(lhs->get_attr(itr->second)->data,
					lhs, global->new_list(List{ rhs }), global->const_null));
			}
			else throw std::exception(("Unknown Operator " + argument).c_str());
		}
		else if (command == "CALL")
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

			auto list = List();
			auto dict = Dict();

			argc -= keywords.size();
			for (auto keyword : keywords)
			{
				dict.push_back(std::make_pair(global->new_str(keyword), stack.top()));
				stack.pop();
			}
			{
				std::list<YPtr> buffer;
				for (; argc > 1; --argc)
				{
					buffer.push_front(stack.top());
					stack.pop();
				}
				for (auto arg : buffer)
				{
					list.push_back(arg);
				}
			}

			YPtr obj = stack.top(); stack.pop();
			if (CompareType(obj->type, global->type_built_in_function))
			{
				if (obj->data != nullptr)
				{
					stack.push((*(BuiltInFunction*)obj->data)(global, global->new_list(list), global->new_dict(dict)));
				}
				else throw std::exception("Unknown Built in Function ");
			}
			else if (CompareType(obj->type, global->type_type))
			{
				auto new_obj = (*(BuiltInFunction*)obj->get_attr("__new__")->data)(global, global->const_null, global->const_null);
				stack.push((*(BuiltInFunction*)new_obj->get_attr("__init__")->data)(global, global->new_list(list), global->new_dict(dict)));
			}
			else throw std::exception("This type is not callable");
		}
		else if (command == "CALL_METHOD")
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
			String method_name = *keywords.begin();
			keywords.pop_front();

			auto list = List();
			auto dict = Dict();

			argc -= keywords.size() + 1;
			for (auto keyword : keywords)
			{
				dict.push_back(std::make_pair(global->new_str(keyword), stack.top()));
				stack.pop();
			}
			{
				std::list<YPtr> buffer;
				for (; argc > 1; --argc)
				{
					buffer.push_front(stack.top());
					stack.pop();
				}
				for (auto arg : buffer)
				{
					list.push_back(arg);
				}
			}

			YPtr self = stack.top(); stack.pop();
			YPtr method = self->get_attr(method_name);
			if (method->type == global->type_built_in_method)
			{
				if (method->data != nullptr)
				{
					stack.push((*(BuiltInMethod*)method->data)(global, self, global->new_list(list), global->new_dict(dict)));
				}
				else throw std::exception("Unknown Built in Method ");
			}
			else throw std::exception("This type is not method");
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
			std::deque<YPtr> args;
			YPtr obj = global->const_null;
			uint64_t dimention = std::stoull(argument) - 1;
			for (uint64_t k = dimention;; --k)
			{
				if (k > 0)
				{
					args.push_front(stack.top());
					stack.pop();
				}
				else
				{
					obj = stack.top();
					stack.pop();
					break;
				}
			}

			auto list = List();
			for (auto arg : args) list.push_back(arg);

			stack.push((*(BuiltInMethod*)obj->get_attr("__getitem__")->data)(global, obj, global->new_list(list), global->const_null));
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