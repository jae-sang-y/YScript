#include "yscript.hpp"

using namespace YScript;

//#define DEBUG_LOGIC_BUILDER
//#define DEBUG_LOGIC_BUILDER_TREE

bool LogicBuilder::next_or_exit(const std::list<Token>& tokens) {
	++itr;
	if (itr == tokens.cend())
		return true;
	now = (*itr);
#ifdef DEBUG_LOGIC_BUILDER
	std::cout << now.str << " ";
	if (now.str == ";") std::cout << '\n';
#endif
	return false;
}

LogicBuilder::LogicBuilder(const std::list<Token>& tokens) {
#ifdef DEBUG_LOGIC_BUILDER
	for (const Token& token : tokens)
	{
		std::cout << token.str << " ";
		if (token.str == ";") std::cout << '\n';
	}
	std::cout << '\n' << "===========================" << '\n';
#endif

	itr = tokens.cbegin();
	now = (*itr);

	//try
	{
		while (itr != tokens.cend())
		{
			if (now.type == TokenType::KeyWord)
			{
				if (now.str == "function")
				{
					auto func_model = logic.push_back(Token{ TokenType::KeyWord, "function" });

					if (next_or_exit(tokens)) throw std::exception("Unexcepted EOF");
					if (now.type != TokenType::Value) throw std::exception("Unexcepted Identifier");
					func_model->push_back(now);

					if (next_or_exit(tokens)) throw std::exception("Unexcepted EOF");;
					if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Identifier");

					auto arguments = func_model->push_back(Token{ TokenType::Structure, "arguments" });

					std::vector<Token> argument_infomation = {};
					while (true)
					{
						if (next_or_exit(tokens)) throw std::exception("Unexcepted EOF");
						if (now == Token{ TokenType::Operator, ")" }) {
							if (argument_infomation.size() == 0) break;
							if (argument_infomation.size() == 1) arguments->push_back(argument_infomation.at(0));
							else throw std::exception("Unexcepted Syntax");
							break;
						}
						if (now == Token{ TokenType::Operator, "," }) {
							if (argument_infomation.size() == 1) arguments->push_back(argument_infomation.at(0));
							else throw std::exception("Unexcepted Syntax");

							argument_infomation.clear();
							continue;
						}
						argument_infomation.push_back(now);
					}

					if (next_or_exit(tokens) || now != Token{ TokenType::Operator, "{" }) throw std::exception("Unexcepted EOF");

					std::stack<tree<Token>*> stack;
					std::vector<Token> expr_list;
					auto body = func_model->push_back(Token{ TokenType::Structure, "body" });
					stack.push(body);

					while (stack.size())
					{
						if (next_or_exit(tokens)) goto exit;

						if (now == Token{ TokenType::KeyWord, "if" }) {
							auto keyword = stack.top()->push_back(now);
							auto* if_statement = keyword->push_back({ TokenType::Structure, "if-statement" });

							if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
							if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Syntax");
							if (expr_list.size()) throw std::exception("Unexcepted Syntax");

							while (true)
							{
								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now == Token{ TokenType::Operator, ")" }) break;
								expr_list.push_back(now);
							}

							if (expr_list.size())
							{
								ExpressionEncoder ee{ expr_list, if_statement };
								expr_list.clear();
							}

#ifdef DEBUG_LOGIC_BUILDER
							std::cout << "\n==if-body==\n";
#endif
							stack.push(keyword->push_back({ TokenType::Structure, "if-body" }));
							keyword->push_back({ TokenType::Structure, "else-body" });
						}
						else if (now == Token{ TokenType::KeyWord, "else" })
						{
							if ((*stack.top()->childs.rbegin())->body == Token{ TokenType::KeyWord,"if" })
							{
#ifdef DEBUG_LOGIC_BUILDER
								std::cout << "\n==else-body==\n";
#endif
								stack.push(*(++++(*stack.top()->childs.rbegin())->childs.begin()));
							}
							else throw std::exception("Unexcepted Syntax");
						}
						else if (now == Token{ TokenType::KeyWord, "do" }) {
							auto keyword = stack.top()->push_back(now);
							keyword->push_back({ TokenType::Structure, "do-statement" });
#ifdef DEBUG_LOGIC_BUILDER
							std::cout << "\n==do-body==\n";
#endif
							stack.push(keyword->push_back({ TokenType::Structure, "do-body" }));
						}
						else if (now == Token{ TokenType::KeyWord, "while" }) {
							if (auto do_root = (*stack.top()->childs.rbegin()); do_root->body == Token{ TokenType::KeyWord,"do" })
							{
								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Syntax");
								if (expr_list.size()) throw std::exception("Unexcepted Syntax");

								while (true)
								{
									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now == Token{ TokenType::Operator, ")" }) break;
									expr_list.push_back(now);
								}

								if (expr_list.size())
								{
									ExpressionEncoder ee{ expr_list , (*do_root->childs.begin()) };
									expr_list.clear();
								}

								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now != Token{ TokenType::Operator, ";" }) throw std::exception("Unexcepted Syntax");
								stack.top()->push_back(now);
							}
							else
							{
								auto do_root2 = (*stack.top()->childs.rbegin());
								auto keyword = stack.top()->push_back(now);
								auto* while_statement = keyword->push_back({ TokenType::Structure, "while-statement" });

								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Syntax");
								if (expr_list.size()) throw std::exception("Unexcepted Syntax");
								while (true)
								{
									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now == Token{ TokenType::Operator, ")" }) break;
									expr_list.push_back(now);
								}

								if (expr_list.size())
								{
									ExpressionEncoder ee{ expr_list, while_statement };
									expr_list.clear();
								}

#ifdef DEBUG_LOGIC_BUILDER
								std::cout << "\n==while-body==\n";
#endif
								stack.push(keyword->push_back({ TokenType::Structure, "while-body" }));
							}
						}
						else if (now == Token{ TokenType::KeyWord, "for" }) {
							auto keyword = stack.top()->push_back(now);
							auto* init_expr = keyword->push_back({ TokenType::Structure, "for-init-statement" });
							auto* cnd_expr = keyword->push_back({ TokenType::Structure, "for-condition-statement" });
							auto* incr_expr = keyword->push_back({ TokenType::Structure, "for-increment-statement" });

							if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
							if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Syntax");

							if (expr_list.size()) throw std::exception("Unexcepted Syntax");
							while (true)
							{
								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now == Token{ TokenType::Operator, ";" }) break;
								expr_list.push_back(now);
							}
							if (expr_list.size())
							{
								ExpressionEncoder ee{ expr_list, init_expr };
								expr_list.clear();
							}

							while (true)
							{
								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now == Token{ TokenType::Operator, ";" }) break;
								expr_list.push_back(now);
							}
							if (expr_list.size())
							{
								ExpressionEncoder ee{ expr_list, cnd_expr };
								expr_list.clear();
							}

							while (true)
							{
								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now == Token{ TokenType::Operator, ")" }) break;
								expr_list.push_back(now);
							}
							if (expr_list.size())
							{
								ExpressionEncoder ee{ expr_list, incr_expr };
								expr_list.clear();
							}
#ifdef DEBUG_LOGIC_BUILDER
							std::cout << "\n==body==\n";
#endif
							stack.push(keyword->push_back({ TokenType::Structure, "for-body" }));
						}
						else if (now == Token{ TokenType::Operator, "{" }) {
#ifdef DEBUG_LOGIC_BUILDER
							std::cout << "\n=={}==\n";
#endif
							stack.push(stack.top()->push_back(Token{ TokenType::Structure, "{}" }));
						}
						else if (now == Token{ TokenType::Operator, "}" }) {
							if (stack.top()->body == Token{ TokenType::Structure, "{}" })
							{
#ifdef DEBUG_LOGIC_BUILDER
								std::cout << "\n==}{==\n";
#endif
								stack.pop();
								if (stack.top()->body == Token{ TokenType::Structure, "for-body" } ||
									stack.top()->body == Token{ TokenType::Structure, "do-body" } ||
									stack.top()->body == Token{ TokenType::Structure, "while-body" } ||
									stack.top()->body == Token{ TokenType::Structure, "if-body" } ||
									stack.top()->body == Token{ TokenType::Structure, "else-body" })
								{
#ifdef DEBUG_LOGIC_BUILDER
									std::cout << "\n===pop===\n";
#endif
									stack.pop();
								}
							}
							else
							{
#ifdef DEBUG_LOGIC_BUILDER
								std::cout << "\n===pop===\n";
#endif
								stack.pop();
							}
						}
						else if (now == Token{ TokenType::Operator, ";" }) {
							if (expr_list.size() > 0)
							{
								expr_list.push_back(now);
								ExpressionEncoder ee{ expr_list, stack.top() };
								expr_list.clear();
							}
							if (stack.top()->body == Token{ TokenType::Structure, "for-body" } ||
								stack.top()->body == Token{ TokenType::Structure, "do-body" } ||
								stack.top()->body == Token{ TokenType::Structure, "while-body" } ||
								stack.top()->body == Token{ TokenType::Structure, "if-body" } ||
								stack.top()->body == Token{ TokenType::Structure, "else-body" })
							{
#ifdef DEBUG_LOGIC_BUILDER
								std::cout << "\n===pop===\n";
#endif
								stack.pop();
							}
#ifdef DEBUG_LOGIC_BUILDER
							std::cout << "\n==;==\n";
#endif
						}
						else {
							expr_list.push_back(now);
						}
					}
#ifdef DEBUG_LOGIC_BUILDER
					std::cout << "\n===out of function===\n";
#endif
				}
			}
			if (next_or_exit(tokens)) goto exit;
		};
	}
	//catch (std::exception e)
	//{
	//	std::cout << "Error!" << '\n';
	//}
exit:;
{
#ifdef DEBUG_LOGIC_BUILDER_TREE
	debug_tree::TreeAnalyzeData* tad = NEW debug_tree::TreeAnalyzeData();
	uint64_t y = 0;
	logic.analyze(tad, 0, y);

	tad->Draw(std::cout);
	delete tad;
#endif
}
}