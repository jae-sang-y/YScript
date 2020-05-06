#include "yscript.hpp"
#include <Windows.h>

using namespace YScript;

//#define DEBUG_ASSEMBLER_BUILDING
//#define DEBUG_ASSEMBLER

void Assembler::process_expression(const tree<Token>* expr, uint64_t depth, uint64_t passing = 0, uint64_t count = -1)
{
	for (auto logic : expr->childs)
	{
		if (passing > 0) { --passing; continue; }
		if (count > 0)
			--count;
		else break;
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
				if (logic->body.str == ";")
					bytecodes.push_back("CLEAR_STACK\t");
				else
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
					bytecodes.push_back("BUILD_DICT");
					process_expression(logic, depth + 1);
				}
				else if (logic->body.str == "List")
				{
					process_expression(logic, depth + 1);
					bytecodes.push_back("BUILD_LIST\t" + std::to_string(logic->childs.size()));
				}
				else if (logic->body.str == "Tuple")
				{
					bytecodes.push_back("BUILD_TUPLE");
					process_expression(logic, depth + 1);
				}
				else if (logic->body.str == "Set")
				{
					bytecodes.push_back("BUILD_SET");
					process_expression(logic, depth + 1);
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
					if (auto func = *logic->childs.begin(); func->body == Token{ TokenType::Structure, "GetAttribute" })
					{
						String call_argument = std::to_string(logic->childs.size() + 1) + ";" + (*std::next(func->childs.begin()))->body.str;
						for (auto child : logic->childs)
							if (child->body == Token{ TokenType::Structure, "FUNCTION_KEYWORD" })
								call_argument += ";" + (*child->childs.begin())->body.str;
						process_expression(func, depth + 1, 0, 1);
						process_expression(logic, depth + 1, 1);
						bytecodes.push_back("CALL_METHOD\t" + call_argument + ";");
					}
					else
					{
						String call_argument = std::to_string(logic->childs.size());
						for (auto child : logic->childs)
							if (child->body == Token{ TokenType::Structure, "FUNCTION_KEYWORD" })
								call_argument += ";" + (*child->childs.begin())->body.str;

						process_expression(logic, depth + 1);
						bytecodes.push_back("CALL\t" + call_argument + ";");
					}
				}
				else if (logic->body.str == "FUNCTION_KEYWORD")
				{
					process_expression(logic, depth + 1, 1);
				}
				else if (logic->body.str == "GetAttribute")
				{
					process_expression(logic, depth + 1, 0, 1);
					bytecodes.push_back("GET_ATTR\t" + (*std::next(logic->childs.begin()))->body.str);
				}
				else if (logic->body.str == "SetAttribute")
				{
					process_expression(logic, depth + 1, 0, 1);
					bytecodes.push_back("SET_ATTR\t" + (*std::next(logic->childs.begin()))->body.str);
				}
				else if (logic->body.str == "GetSubscript")
				{
					process_expression(logic, depth + 1);
					bytecodes.push_back("GET_SUBSRPT\t" + std::to_string(logic->childs.size()));
				}
				else if (logic->body.str == "SetSubscript")
				{
					process_expression(logic, depth + 1);
					bytecodes.push_back("SET_SUBSRPT\t" + std::to_string(logic->childs.size()));
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
					bytecodes.push_back("CLEAR_STACK\t");

					uint64_t for_start_index = bytecodes.size();
					process_expression(condition_statement, depth + 1);
					uint64_t if_false_jump_index = bytecodes.size();
					bytecodes.push_back("IF_FALSE_JUMP\t");
					process_expression(body_expression, depth + 1);
					process_expression(increment_statement, depth + 1);
					bytecodes.push_back("CLEAR_STACK\t");
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
					bytecodes.push_back("CLEAR_STACK\t");
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
#ifdef DEBUG_ASSEMBLER_BUILDING
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
#ifdef DEBUG_ASSEMBLER
	for (auto bytecode : bytecodes)
	{
		std::cout << bytecode << "\r\n";
	}
#endif

	return;
		}