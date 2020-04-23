#include "yscript.hpp"

using namespace YScript;

//#defube DEBUG_ASSEMBLER
//#define DEBUG_EXECUTER

YObject* literal_decode(const std::string& src)
{
	if (src == "true")
	{
		return new YObject(YObjectType::Bool, new bool(true));
	}
	else if (src == "false")
	{
		return new YObject(YObjectType::Bool, new bool(false));
	}
	else if (src.at(0) == '"')
	{
		std::string* str = new std::string();
		uint64_t k = 0;
		uint64_t passing_count = 1;
		for (std::string::const_iterator csr = src.cbegin(); csr != src.cend(); ++csr, ++k)
		{
			if (k == src.size() - 1) continue;
			if (passing_count) --passing_count;
			for (const auto& pair : __string_literal_pairs__)
			{
				if (Lexer::match(pair.first, csr, src.cend()))
				{
					str->push_back(pair.second);
					goto outer_continue;
				}
			}
			str->push_back(*csr);
		outer_continue:;
		}
		return new YObject(YObjectType::String, str);
	}
	else
	{
		if (src.find('.') == -1)
		{
			return new YObject(YObjectType::I32, new int32_t(std::stof(src)));
		}
		else
		{
			return new YObject(YObjectType::F32, new float(std::stoi(src)));
		}
	}
}

std::string literal_encode(const std::string& src)
{
	std::string a_return = "";
	if (src.size() > 2)
	{
		uint64_t k = 0;
		a_return.push_back('"');
		for (const char& ch : src)
		{
			if (k == 0 || k == src.size() - 1) {
				k += 1;
				continue;
			}
			for (const auto& pair : __string_literal_pairs__)
			{
				if (ch == pair.second)
				{
					a_return += pair.first;
					goto outer_continue;
				}
			}
			++k;
			a_return.push_back(ch);
		outer_continue:;
		}
		a_return.push_back('"');
	}
	else a_return = src;

	return a_return;
}

std::string GetRepr(YObject* obj)
{
	switch (obj->type_id)
	{
	case YObjectType::I32: return std::to_string(*(int32_t*)obj->data);
	case YObjectType::F32: return std::to_string(*(float*)obj->data);
	case YObjectType::Bool: return (*(bool*)obj->data) ? "true" : "false";
	case YObjectType::Function: return "Function 0x" + std::to_string(*(uint64_t*)obj->data);
	case YObjectType::String: return "\'" + literal_encode(*(std::string*)obj->data) + "\'";
	}
	return "YObject 0x" + std::to_string(obj->object_id);
}

YObject* convert(YObject* obj, const YObjectType target)
{
	if (target == YObjectType::Bool)
	{
		bool data = true;
		if (obj->type_id == YObjectType::Null)
			data = false;
		else if (obj->type_id == YObjectType::Bool)
			data = *(bool*)obj->data;
		else if (obj->type_id == YObjectType::I32 && (*(int32_t*)obj->data == 0))
			data = false;
		return new YObject(target, new bool(data));
	}
	else if (target == YObjectType::F32)
	{
		float data = 0.f;
		if (obj->type_id == YObjectType::I32) data = (float)*(int32_t*)obj->data;
		else throw std::exception("Unexpected src type");
		return new YObject(target, new float(data));
	}
	else if (target == YObjectType::String)
	{
		return new YObject(target, new std::string(GetRepr(obj)));
	}
	else throw std::exception("Unexpected target type");
	do {} while (true);
}

//YObject* mono_operand(const std::string& op, const YObject* target)
//{
//	if (op == "!" && target->type_id == YObjectType::Bool)
//	{
//		//return new YO
//	}
//}
YObject* deepcopy(const YObject* src)
{
	if (src->type_id == YObjectType::Bool) return new YObject(YObjectType::Bool, new bool(*(bool*)src->data));
	if (src->type_id == YObjectType::I32) return new YObject(YObjectType::I32, new int32_t(*(int32_t*)src->data));
	if (src->type_id == YObjectType::F32) return new YObject(YObjectType::F32, new float(*(float*)src->data));
	else throw std::exception("Unexpected Type");
}

YObject* operand(const std::string& op, YObject* lhs, YObject* rhs)
{
	YObject* result = nullptr;
	if (op == "+" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::I32, new int32_t(*(int32_t*)lhs->data + *(int32_t*)rhs->data));
	else if (op == "-" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::I32, new int32_t(*(int32_t*)lhs->data - *(int32_t*)rhs->data));
	else if (op == "*" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::I32, new int32_t(*(int32_t*)lhs->data * *(int32_t*)rhs->data));
	else if (op == "/" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::I32, new int32_t(*(int32_t*)lhs->data / *(int32_t*)rhs->data));
	else if (op == "%" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::I32, new int32_t(*(int32_t*)lhs->data % *(int32_t*)rhs->data));
	else if (op == "**" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
	{
		int32_t buffer = 1;
		for (uint64_t k = 0; k < *(int32_t*)rhs->data; ++k)
			buffer *= *(int32_t*)lhs->data;
		result = new YObject(YObjectType::I32, new int32_t(buffer));
	}
	else if (op == ">" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::Bool, new bool(*(int32_t*)lhs->data > * (int32_t*)rhs->data));
	else if (op == "<" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::Bool, new bool(*(int32_t*)lhs->data < *(int32_t*)rhs->data));
	else if (op == "==" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::Bool, new bool(*(int32_t*)lhs->data == *(int32_t*)rhs->data));
	else if (op == "!=" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		result = new YObject(YObjectType::Bool, new bool(*(int32_t*)lhs->data != *(int32_t*)rhs->data));
	else if (op == ">=")
		result = new YObject(YObjectType::Bool, new bool(!operand("<", lhs, rhs)));
	else if (op == "<=")
		result = new YObject(YObjectType::Bool, new bool(!operand(">", lhs, rhs)));

	else if (op == "+" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::F32, new float(*(float*)lhs->data + *(float*)rhs->data));
	else if (op == "-" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::F32, new float(*(float*)lhs->data - *(float*)rhs->data));
	else if (op == "*" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::F32, new float(*(float*)lhs->data * *(float*)rhs->data));
	else if (op == "/" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::F32, new float(*(float*)lhs->data / *(float*)rhs->data));
	else if (op == "**" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::F32, new float(powf(*(float*)lhs->data, *(float*)rhs->data)));
	else if (op == ">" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::Bool, new bool(*(float*)lhs->data > * (float*)rhs->data));
	else if (op == "<" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::Bool, new bool(*(float*)lhs->data < *(float*)rhs->data));
	else if (op == "==" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::Bool, new bool(*(float*)lhs->data == *(float*)rhs->data));
	else if (op == "!=" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
		result = new YObject(YObjectType::Bool, new bool(*(float*)lhs->data != *(float*)rhs->data));
	else if ((op == "+" || op == "-" || op == "*" || op == "/" || op == "**" ||
		op == "==" || op == "!=" || op == ">" || op == "<")
		&& (lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::I32 ||
			lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::F32)
		)
	{
		if (lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::I32) result = operand(op, lhs, convert(rhs, YObjectType::F32));
		else result = operand(op, convert(lhs, YObjectType::F32), rhs);
	}
	else throw std::exception(("Unsupport Operator for : " + std::to_string(lhs->type_id) + " " + op + " " + std::to_string(rhs->type_id)).c_str());
	delete lhs;
	delete rhs;
	return result;
}

std::string Print(YObject** obj, size_t number)
{
	std::string result = "";
	for (size_t k = 0; k < number; ++k)
	{
		result += GetRepr(obj[k]);
		result.push_back('\r');
		result.push_back('\n');
	}
	return result;
}

void Assembler::process_expression(const tree<Token>* expr, uint64_t depth, uint64_t passing = 0)
{
	for (auto logic : expr->childs)
	{
		if (passing > 0) { --passing; continue; }
		/*if (logic->body == Token{ TokenType::Structure, "Define" })
		{
			process_expression((*std::next(logic->childs.begin())), depth + 1);
			bytecodes.push_back("STORE_VALUE\t" + (*logic->childs.begin())->body.str);
		}
		else*/
		{
			switch (logic->body.type)
			{
			case TokenType::Value:
				bytecodes.push_back("PUSH_VALUE\t" + logic->body.str);
				break;
			case TokenType::Literal:
				bytecodes.push_back("PUSH_LITERAL\t" + literal_encode(logic->body.str));
				break;
			case TokenType::Operator:
				if (logic->body.str != ";")
					throw std::exception("Expr builder is corrupted");
				break;
			case TokenType::Caculation:
				process_expression(logic, depth + 1);
				bytecodes.push_back("OPERATOR\t" + logic->body.str);
				break;
			case TokenType::Structure:
				if (logic->body.str == "{}" || logic->body.str == "()")
				{
					process_expression(logic, depth + 1);
				}
				else if (logic->body.str == "Dict")
				{
				}
				else if (logic->body.str == "Assign")
				{
					process_expression(logic, depth + 1, 1);
					bytecodes.push_back("STORE_VALUE\t" + (*logic->childs.begin())->body.str);
				}
				else if (logic->body.str == "Inplace_Add")
				{
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
					process_expression(logic, depth + 1, 1);
					bytecodes.push_back("OPERATOR\t+");
					bytecodes.push_back("STORE_VALUE\t" + (*logic->childs.begin())->body.str);
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
				}
				else if (logic->body.str == "Inplace_Sub")
				{
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
					process_expression(logic, depth + 1, 1);
					bytecodes.push_back("OPERATOR\t-");
					bytecodes.push_back("STORE_VALUE\t" + (*logic->childs.begin())->body.str);
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
				}
				else if (logic->body.str == "Inplace_Mul")
				{
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
					process_expression(logic, depth + 1, 1);
					bytecodes.push_back("OPERATOR\t*");
					bytecodes.push_back("STORE_VALUE\t" + (*logic->childs.begin())->body.str);
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
				}
				else if (logic->body.str == "Inplace_Div")
				{
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
					process_expression(logic, depth + 1, 1);
					bytecodes.push_back("OPERATOR\t/");
					bytecodes.push_back("STORE_VALUE\t" + (*logic->childs.begin())->body.str);
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
				}
				else if (logic->body.str == "Inplace_Mod")
				{
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
					process_expression(logic, depth + 1, 1);
					bytecodes.push_back("OPERATOR\t%");
					bytecodes.push_back("STORE_VALUE\t" + (*logic->childs.begin())->body.str);
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
				}
				else if (logic->body.str == "Inplace_Pow")
				{
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
					process_expression(logic, depth + 1, 1);
					bytecodes.push_back("OPERATOR\t**");
					bytecodes.push_back("STORE_VALUE\t" + (*logic->childs.begin())->body.str);
					bytecodes.push_back("PUSH_VALUE\t" + (*logic->childs.begin())->body.str);
				}
				else if (logic->body.str == "Call")
				{
					process_expression(logic, depth + 1);
					bytecodes.push_back("CALL\t" + std::to_string(logic->childs.size()));
				}
				else
				{
					throw std::exception();
				}
				break;
				/*case TokenType:::
					bytecodes.push_back("CACULATE " + logic->body.str);
					break;
				*/
			case TokenType::KeyWord:
				if (logic->body.str == "if")
				{
					const tree<Token>* if_statement = *logic->childs.begin();
					const tree<Token>* if_expression = *std::next(logic->childs.begin());
					const tree<Token>* else_expression = *std::next(logic->childs.begin(), 2);
					process_expression(if_statement, depth + 1);

					bytecodes.push_back("IF_FALSE_JUMP\t");
					uint64_t if_false_jump_index = bytecodes.size() - 1;

					process_expression(if_expression, depth + 1);
					auto& if_false_jump_target = bytecodes.at(if_false_jump_index);
					if_false_jump_target.append(std::to_string(bytecodes.size() + 1));

					bytecodes.push_back("JUMP_TO\t");
					uint64_t jump_index = bytecodes.size() - 1;

					process_expression(else_expression, depth + 1);
					auto& jump_target = bytecodes.at(jump_index);
					jump_target.append(std::to_string(bytecodes.size()));
				}
				else if (logic->body.str == "for")
				{
					const tree<Token>* init_expression = *logic->childs.begin();
					const tree<Token>* condition_statement = *std::next(logic->childs.begin());
					const tree<Token>* increment_statement = *std::next(logic->childs.begin(), 2);
					const tree<Token>* body_expression = *std::next(logic->childs.begin(), 3);
					process_expression(init_expression, depth + 1);

					uint64_t for_start_index = bytecodes.size();
					process_expression(condition_statement, depth + 1);
					uint64_t if_false_jump_index = bytecodes.size();
					bytecodes.push_back("IF_FALSE_JUMP\t");
					process_expression(body_expression, depth + 1);
					process_expression(increment_statement, depth + 1);
					bytecodes.push_back("JUMP_TO\t" + std::to_string(for_start_index));
					bytecodes.at(if_false_jump_index) += std::to_string(bytecodes.size());
				}
				else if (logic->body.str == "while")
				{
					const tree<Token>* while_statement = *logic->childs.begin();
					const tree<Token>* while_body = *std::next(logic->childs.begin());
					uint64_t while_start_index = bytecodes.size();
					process_expression(while_statement, depth + 1);
					uint64_t if_false_jump_index = bytecodes.size();
					bytecodes.push_back("IF_FALSE_JUMP\t");
					process_expression(while_body, depth + 1);
					bytecodes.push_back("JUMP_TO\t" + std::to_string(while_start_index));
					bytecodes.at(if_false_jump_index) += std::to_string(bytecodes.size());
				}
				else if (logic->body.str == "do")
				{
					const tree<Token>* do_statement = *logic->childs.begin();
					const tree<Token>* do_body = *std::next(logic->childs.begin());
					uint64_t do_start_index = bytecodes.size();
					process_expression(do_body, depth + 1);
					process_expression(do_statement, depth + 1);
					uint64_t if_false_jump_index = bytecodes.size();
					bytecodes.push_back("IF_FALSE_JUMP\t");
					bytecodes.push_back("JUMP_TO\t" + std::to_string(do_start_index));
					bytecodes.at(if_false_jump_index) += std::to_string(bytecodes.size());
				}
				else
				{
					bytecodes.push_back("KEYWORD\t" + logic->body.str);
				}
				break;
			default:
				bytecodes.push_back(logic->body.str);
				break;
			}
		}
	}
}

Assembler::Assembler(const tree<Token>& logic)
{
	for (auto child : logic.childs)
	{
		if (child->body == Token{ TokenType::KeyWord, "function" })
		{
#ifdef DEBUG_ASSEMBLER
			std::cout << "===function name===" << (*child->childs.begin())->body.str << '\n';
			std::cout << "===function arguments===" << '\n';

			for (auto argument : (*std::next(child->childs.begin()))->childs)
			{
				std::cout << "===argument " << argument->body.str << '\n';
			}

			std::cout << "===function body===" << '\n';
#endif
			process_expression((*std::next(child->childs.begin(), 2)), 0);
			bytecodes.push_back("CLEAR_STACK\t");
		}
	}

	std::stack<YObject*> stack;
	std::map<uint64_t, std::map<std::string, YObject*>> value_map;
	uint64_t object_id = 0;

	value_map[5]["print"] = new YObject(YObjectType::Function, new uint64_t(0x01));

	for (uint64_t k = 0; k < bytecodes.size(); ++k)
	{
		const std::string command = bytecodes[k].substr(0, bytecodes[k].find('\t'));
		const std::string argument = bytecodes[k].substr(bytecodes[k].find('\t') + 1);
#ifdef DEBUG_EXECUTER
		std::cout << (1000000 + k) << "  " << command << "\t" << argument << '\n';
#endif
		}
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
			stack.top()->is_rightvalue = false;
			value_map[argument.size()][argument] = stack.top();
			stack.pop();
		}
		else if (command == "PUSH_VALUE")
		{
			if (auto itr = value_map.find(argument.size()); itr == value_map.end())
				throw std::exception(("Undefined Variable " + argument).c_str());
			if (auto page = value_map[argument.size()]; page.find(argument) == page.end())
				throw std::exception(("Undefined Variable " + argument).c_str());

			stack.push(value_map[argument.size()][argument]);
		}
		else if (command == "IF_FALSE_JUMP")
		{
			auto data = *(bool*)convert(stack.top(), YObjectType::Bool)->data;
			if (!data)
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
				auto rhs = stack.top(); stack.pop();
				auto lhs = stack.top(); stack.pop();

				stack.push(operand(argument, deepcopy(lhs), deepcopy(rhs)));
				if (lhs->is_rightvalue) delete lhs;
				if (rhs->is_rightvalue) delete rhs;
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
				std::cout << Print(arguments.data(), arguments.size());
			}
			else throw std::exception(("Unknown Function " + std::to_string(function_id)).c_str());
		}

#ifdef DEBUG_EXECUTER
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
		}
	return;
	}