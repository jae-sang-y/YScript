#include "yscript.hpp"

using namespace YScript;

//#define DEBUG_EXPRESSION_ENCODER

ExpressionEncoder::ExpressionEncoder(std::vector<Token> tokens, tree<YScript::Token>* target) {
#ifdef DEBUG_EXPRESSION_ENCODER
	std::cout << '\n' << "===========================" << '\n';
	for (const Token& token : tokens)
	{
		std::cout << token.str << " ";
		if (token.str == ";") std::cout << '\n';
	}
#endif
	tree<Token> expr_tree{ Token{TokenType::Structure, "root"} };
	read_brackets(tokens, &expr_tree);
	for (auto itr = expr_tree.childs.begin(); itr != expr_tree.childs.end(); ++itr)
	{
		target->childs.push_back(*itr);
		(*itr) = nullptr;
	}

#ifdef DEBUG_EXPRESSION_ENCODER
	debug_tree::TreeAnalyzeData* tad = new debug_tree::TreeAnalyzeData();
	size_t y = 0;
	tad->max_height = expr_tree.get_depth(1);
	expr_tree.analyze(tad, 0, y);
	tad->Draw(std::cout);
	delete tad;
#endif
}

void ExpressionEncoder::read_brackets(std::vector<Token> tokens, tree<Token>* root) {
	std::list<tree<Token>*> stack;
	stack.push_back(root);
#ifdef DEBUG_EXPRESSION_ENCODER
	std::cout << std::endl;
#endif

	for (Token& token : tokens)
	{
		if (token == Token{ TokenType::Operator, "(" })
		{
			stack.push_back((*stack.rbegin())->push_back(Token{ TokenType::Structure, "()" }));
		}
		else if (token == Token{ TokenType::Operator, "[" })
		{
			stack.push_back((*stack.rbegin())->push_back(Token{ TokenType::Structure, "[]" }));
		}
		else if (token == Token{ TokenType::Operator, "{" })
		{
			stack.push_back((*stack.rbegin())->push_back(Token{ TokenType::Structure, "{}" }));
		}
		else if (token == Token{ TokenType::Operator, ")" })
		{
			if ((*stack.rbegin())->body != Token{ TokenType::Structure, "()" })
				throw std::exception("Unexcepted Syntax");
			read_operators((*stack.rbegin() - 1), (*stack.rbegin()), ')');
			stack.pop_back();
		}
		else if (token == Token{ TokenType::Operator, "]" })
		{
			if ((*stack.rbegin())->body != Token{ TokenType::Structure, "[]" })
				throw std::exception("Unexcepted Syntax");
			read_operators((*stack.rbegin() - 1), (*stack.rbegin()), ']');
			stack.pop_back();
		}
		else if (token == Token{ TokenType::Operator, "}" })
		{
			if ((*stack.rbegin())->body != Token{ TokenType::Structure, "{}" })
				throw std::exception("Unexcepted Syntax");
			read_operators((*stack.rbegin() - 1), (*stack.rbegin()), '}');
			stack.pop_back();
		}
		else (*stack.rbegin())->push_back(token);
	}
	read_operators((*stack.rbegin() - 1), (*stack.rbegin()), '\0');
}

tree<Token>* ExpressionEncoder::read_operators(tree<Token>* parent, tree<Token>* root, const char& end_token)
{
#ifdef DEBUG_EXPRESSION_ENCODER
	for (tree<Token> const* child : root->childs)
	{
		std::cout << child->body.str << "   ";
	}
	std::cout << std::endl;
#endif

	for (auto itr = root->childs.begin(); itr != root->childs.end(); ++itr)
	{
		tree<Token>* child = (*itr);
		if (child->body == Token{ TokenType::Operator, "." })
		{
			child->body.type = TokenType::Caculation;
			tree<Token>* left = *std::prev(itr);
			tree<Token>* right = *std::next(itr);

			child->childs.push_back(left);
			child->childs.push_back(right);

			root->childs.erase(std::prev(itr));
			root->childs.erase(std::next(itr));
		}
	}

	for (auto itr = root->childs.begin(); itr != root->childs.end(); ++itr)
	{
		tree<Token>* child = (*itr);
		if (child->body == Token{ TokenType::Structure, "()" })
		{
			if (itr != root->childs.begin())
			{
				if (auto prev_itr = std::prev(itr); (*prev_itr)->body.type != TokenType::Operator)
				{
					child->body.str = "call";

					for (auto itr = child->childs.begin(); itr != child->childs.end(); ++itr)
						if ((*itr)->body == Token{ TokenType::Operator, "," })
							child->childs.erase(itr--);

					child->childs.push_front(*prev_itr);

					root->childs.erase(std::prev(itr));
				}
			} //else is_a_normal_brackets
		}
		else if (child->body == Token{ TokenType::Structure, "[]" })
		{
			if (itr != root->childs.begin())
			{
				if (auto prev_itr = std::prev(itr); (*prev_itr)->body.type != TokenType::Operator)
				{
					child->body.str = "subscript";

					for (auto itr = child->childs.begin(); itr != child->childs.end(); ++itr)
						if ((*itr)->body == Token{ TokenType::Operator, "," })
							child->childs.erase(itr--);

					child->childs.push_front(*prev_itr);

					root->childs.erase(std::prev(itr));
				}
			} //else is_a_normal_brackets
		}
	}

	for (const OperatorPrecedence& op_precede : op_precedences)
	{
		if (op_precede.left_to_right)
		{
			for (auto itr = root->childs.begin(); itr != root->childs.end(); ++itr)
			{
				tree<Token>* child = (*itr);
				for (const std::string op : op_precede.operators)
				{
					if (child->body == Token{ TokenType::Operator, op })
					{
						if (op_precede.left_op && op_precede.right_op)
						{
							tree<Token>* left = *std::prev(itr);
							tree<Token>* right = *std::next(itr);

							child->childs.push_back(left);
							child->childs.push_back(right);

							root->childs.erase(std::prev(itr));
							root->childs.erase(std::next(itr));
						}
						else if (op_precede.left_op)
						{
							if (itr != root->childs.begin())
							{
								tree<Token>* left = *std::prev(itr);
								child->childs.push_back(left);
								root->childs.erase(std::prev(itr));
								root->body.str += "-left";
							}
						}
						else if (op_precede.right_op)
						{
							if (std::next(itr) != root->childs.end())
							{
								tree<Token>* right = *std::next(itr);
								child->childs.push_back(right);
								root->childs.erase(std::next(itr));
								root->body.str += "-right";
							}
						}
						break;
					}
				}
			}
		}
		else
		{
			for (auto itr = --root->childs.end();; ++itr)
			{
				tree<Token>* child = (*itr);
				for (const std::string op : op_precede.operators)
				{
					if (child->body == Token{ TokenType::Operator, op })
					{
						if (op_precede.left_op && op_precede.right_op)
						{
							tree<Token>* left = *std::prev(itr);
							tree<Token>* right = *std::next(itr);

							child->childs.push_back(left);
							child->childs.push_back(right);

							root->childs.erase(std::prev(itr));
							root->childs.erase(std::next(itr));
						}
						else if (op_precede.left_op)
						{
							if (itr != root->childs.begin())
							{
								tree<Token>* left = *std::prev(itr);
								child->childs.push_back(left);
								root->childs.erase(std::prev(itr));
								root->body.str += "-left";
							}
						}
						else if (op_precede.right_op)
						{
							if (std::next(itr) != root->childs.end())
							{
								tree<Token>* right = *std::next(itr);
								child->childs.push_back(right);
								root->childs.erase(std::next(itr));
								root->body.str += "-right";
							}
						}
						break;
					}
				}
				if (itr == root->childs.begin()) break;
			}
		}
	}

	for (auto itr = root->childs.begin(); itr != root->childs.end(); ++itr)
	{
		tree<Token>* child = (*itr);
		if (child->body == Token{ TokenType::Operator, ":" })
		{
			if (itr == root->childs.begin())  throw std::exception("Unexcepted Syntax");

			tree<Token>* left = *std::prev(itr);
			tree<Token>* right = *std::next(itr);

			child->childs.push_back(left);
			child->childs.push_back(right);

			root->childs.erase(std::prev(itr));
			root->childs.erase(std::next(itr));
		}
	}

	if (end_token == '}' || end_token == ')')
	{
		for (auto itr = root->childs.begin(); itr != root->childs.end(); ++itr)
		{
			if ((*itr)->body == Token{ TokenType::Operator, "," })
				root->childs.erase(itr--);
		}
		if (end_token == ')')
		{
			root;
			int i = 0;
		}
	}

	return root;
}