#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <unordered_map>

#define DEBUG_TREE
#include "tree.hpp"

/*
String -> [Lexer]
Tokens -> [Logical Tree]
*/

//#define DEBUG_LEXING
#define DEBUG_LOGIC_BUILDER

const static std::list<std::string> __keywords__ = {
	"function",
	"for",
	"while",
	"do",
	"else",
	"if",
	"var",
	"break",
	"continue",
	"yield"
};

const static std::list<std::string> __ops__ = {
	"++",
	"--",
	"==",
	"!=",
	">=",
	"<=",
	"&&",
	"||",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	">",
	"<",
	"!",
	"=",
	"+",
	"-",
	"*",
	"/",
	"%",
	"[",
	"]",
	".",
	"(",
	")",
	",",
	"?",
	":",
	";",
	"{",
	"}",
};

const std::unordered_map<std::string, char> __string_literal_pairs__ = {
	{"\\n" ,'\n'},
	{"\\\\",'\\'},
	{"\\t" ,'\t'},
	{"\\?" ,'\?'},
	{"\\v" ,'\v'},
	{"\\'" ,'\''},
	{"\\b" ,'\b'},
	{"\\\"",'"'},
	{"\\r" ,'\r'},
	{"\\0" ,'\0'},
	{"\\f" ,'\f'},
};

bool match(const std::string& token, std::string::const_iterator itr, std::string::const_iterator end) {
	for (std::string::const_iterator cursor = token.cbegin(); ;)
	{
		if (cursor == token.end())
			return true;

		if (itr == end)
			return false;
		if ((*itr) != (*cursor))
			return false;

		++cursor;
		++itr;
	}
}

bool is_space(char ch)
{
	return (ch == '\r' || ch == '\n' || ch == '\t' || ch == '\f' || ch == '\v' || ch == ' ');
}

enum class LexingState {
	Logic,
	String,
	CommentLine,
	CommentLines
};

enum class TokenType {
	Structure,
	Operator,
	KeyWord,
	Value
};

struct Token {
	TokenType token;
	std::string str;
	bool operator == (const Token& other)
	{
		return token == other.token && str == other.str;
	}
	bool operator != (const Token& other)
	{
		return !(*this == other);
	}
};

class Lexer
{
public:
	std::list<Token> tokens;

	Lexer(std::string::const_iterator begin, std::string::const_iterator end) {
		for (std::string::const_iterator csr = begin; csr != end; ++csr)
		{
			if (passing_count > 0)
			{
				--passing_count;
				continue;
			}

			if (lexing_state == LexingState::Logic)
			{
				if (leftover.size() == 0)
				{
					for (const auto& keyword : __keywords__)
					{
						if (match(keyword, csr, end))
						{
							if (leftover.size())
							{
#ifdef DEBUG_LEXING
								std::cout << "VAL: " << leftover << '\n';
#endif
								tokens.push_back(Token{ TokenType::Value, leftover });
								leftover.clear();
							}
							passing_count += keyword.size() - 1;
#ifdef DEBUG_LEXING
							std::cout << "KEY: " << keyword << '\n';
#endif
							tokens.push_back(Token{ TokenType::KeyWord, keyword });

							goto next_loop;
						}
					}
				}
				for (const auto& op : __ops__)
				{
					if (op.size() > 1 && match(op, csr, end) || op.size() == 1 && *csr == op.at(0))
					{
						if (leftover.size())
						{
#ifdef DEBUG_LEXING
							std::cout << "VAL: " << leftover << '\n';
#endif
							tokens.push_back(Token{ TokenType::Value, leftover });
							leftover.clear();
						}
						passing_count += op.size() - 1;
#ifdef DEBUG_LEXING
						std::cout << "OPE: " << op << '\n';
#endif
						tokens.push_back(Token{ TokenType::Operator, op });
						goto next_loop;
					}
				}

				if (*csr == '"')
				{
					lexing_state = LexingState::String;
					goto next_loop;
				}

				if (match("//", csr, end))
				{
					passing_count += 1;
					lexing_state = LexingState::CommentLine;
					goto next_loop;
				}
				if (match("/*", csr, end))
				{
					passing_count += 1;
					lexing_state = LexingState::CommentLines;
					goto next_loop;
				}
				if (is_space(*csr))
				{
					if (leftover.size())
					{
#ifdef DEBUG_LEXING
						std::cout << "VAL: " << leftover << '\n';
#endif
						tokens.push_back(Token{ TokenType::Value, leftover });
						leftover.clear();
					}
				}
				else
				{
					leftover.push_back(*csr);
				}
			}
			else if (lexing_state == LexingState::String)
			{
				for (const auto pair : __string_literal_pairs__)
				{
					if (match(pair.first, csr, end))
					{
						passing_count += pair.first.size() - 1;
						string_builder.push_back(pair.second);
						goto next_loop;
					}
				}
				if (*csr == '"')
				{
					lexing_state = LexingState::Logic;
#ifdef DEBUG_LEXING
					std::cout << "VAL: " << "\"" << string_builder << "\"" << '\n';
#endif
					tokens.push_back(Token{ TokenType::Value, string_builder });
					string_builder.clear();
					goto next_loop;
				}
				string_builder.push_back(*csr);
			}
			else if (lexing_state == LexingState::CommentLine)
			{
				if (*csr == '\n')
					lexing_state = LexingState::Logic;
			}
			else if (lexing_state == LexingState::CommentLines)
			{
				if (match("*/", csr, end))
				{
					passing_count += 1;
					lexing_state = LexingState::Logic;
				}
			}

		next_loop:;
		}
	}
private:
	LexingState lexing_state = LexingState::Logic;
	size_t passing_count = 0;
	std::string leftover = "";
	std::string string_builder = "";
};

class LogicBuilder {
public:
	LogicBuilder(const std::list<Token>& tokens) {
#ifdef DEBUG_LOGIC_BUILDER
		for (const Token& token : tokens)
		{
			std::cout << token.str << " ";
			if (token.str == ";") std::cout << '\n';
		}
		std::cout << '\n' << "===========================" << '\n';
#endif
		tree<Token> logic{ Token{TokenType::Structure, "root"} };

		itr = tokens.cbegin();
		now = (*itr);

		try {
			while (itr != tokens.cend())
			{
				if (now.token == TokenType::KeyWord)
				{
					if (now.str == "function")
					{
						auto& func_model = logic.push_back(Token{ TokenType::KeyWord, "function" });

						if (next_or_exit(tokens)) throw std::exception("Unexcepted EOF");
						if (now.token != TokenType::Value) throw std::exception("Unexcepted Identifier");
						func_model.push_back(now);

						if (next_or_exit(tokens)) throw std::exception("Unexcepted EOF");;
						if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Identifier");

						auto& arguments = func_model.push_back(Token{ TokenType::Structure, "arguments" });

						std::vector<Token> argument_infomation = {};
						while (true)
						{
							if (next_or_exit(tokens)) throw std::exception("Unexcepted EOF");
							if (now == Token{ TokenType::Operator, ")" }) {
								if (argument_infomation.size() == 0) break;
								if (argument_infomation.size() == 1) arguments.push_back(argument_infomation.at(0));
								else throw std::exception("Unexcepted Syntax");
								break;
							}
							if (now == Token{ TokenType::Operator, "," }) {
								if (argument_infomation.size() == 1) arguments.push_back(argument_infomation.at(0));
								else throw std::exception("Unexcepted Syntax");

								argument_infomation.clear();
								continue;
							}
							argument_infomation.push_back(now);
						}

						if (next_or_exit(tokens) || now != Token{ TokenType::Operator, "{" }) throw std::exception("Unexcepted EOF");

						std::stack<tree<Token>*> stack;
						std::stack<std::string> statement;
						auto& body = func_model.push_back(Token{ TokenType::Structure, "body" });
						stack.push(&body);
						statement.push("function");

						while (stack.size())
						{
							if (next_or_exit(tokens)) goto exit;

							if (now == Token{ TokenType::KeyWord, "var" }) {
								if (next_or_exit(tokens)) throw std::exception("Unexcepted EOF");
								auto& define_var = stack.top()->push_back(Token{ TokenType::Structure, "Define" });
								define_var.push_back(now);

								if (next_or_exit(tokens)) throw std::exception("Unexcepted EOF");
								if (now != Token{ TokenType::Operator, "=" }) throw std::exception("Unexcepted Syntax");

								stack.push(&define_var.push_back(Token{ TokenType::Structure, "var-body" }));
								statement.push("var");
							}
							else if (now == Token{ TokenType::KeyWord, "if" }) {
								auto& keyword = stack.top()->push_back(now);
								auto& expr = keyword.push_back({ TokenType::Structure, "if-statement" });

								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Syntax");

								while (true)
								{
									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now == Token{ TokenType::Operator, ")" }) break;
									expr.push_back(now);
								}

								stack.push(&keyword.push_back({ TokenType::Structure, "if-body" }));
								keyword.push_back({ TokenType::KeyWord, "else-body" });
								statement.push("if");
							}
							else if (now == Token{ TokenType::KeyWord, "else" })
							{
								if (stack.top()->childs.rbegin()->body == Token{ TokenType::KeyWord,"if" })
								{
									stack.push(&stack.top()->childs.rbegin()->childs.at(2));
									statement.push("else");
								}
								else throw std::exception("Unexcepted Syntax");
							}
							else if (now == Token{ TokenType::KeyWord, "do" }) {
								auto& keyword = stack.top()->push_back(now);
								keyword.push_back({ TokenType::Structure, "do-statement" });
								stack.push(&keyword.push_back({ TokenType::Structure, "do-body" }));
								statement.push("do");
							}
							else if (now == Token{ TokenType::KeyWord, "while" }) {
								if (stack.top()->childs.rbegin()->body == Token{ TokenType::Structure,"do" })
								{
									stack.push(stack.top()->childs.rbegin()->childs.begin()._Ptr);

									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Syntax");
									while (true)
									{
										if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
										if (now == Token{ TokenType::Operator, ")" }) break;
										stack.top()->childs.push_back(now);
									}

									stack.pop();

									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now != Token{ TokenType::Operator, ";" }) throw std::exception("Unexcepted Syntax");
									stack.top()->childs.push_back(now);
								}
								else
								{
									auto& keyword = stack.top()->push_back(now);
									auto& expr = keyword.push_back({ TokenType::Structure, "while-statement" });

									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Syntax");
									while (true)
									{
										if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
										if (now == Token{ TokenType::Operator, ")" }) break;
										expr.push_back(now);
									}

									stack.push(&keyword.push_back({ TokenType::KeyWord, "while-body" }));
									statement.push("while");
								}
							}
							else if (now == Token{ TokenType::KeyWord, "for" }) {
								auto& keyword = stack.top()->push_back(now);
								auto& init_expr = keyword.push_back({ TokenType::Structure, "for-init-statement" });
								auto& cnd_expr = keyword.push_back({ TokenType::Structure, "for-condition-statement" });
								auto& incr_expr = keyword.push_back({ TokenType::Structure, "for-increment-statement" });

								if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
								if (now != Token{ TokenType::Operator, "(" }) throw std::exception("Unexcepted Syntax");

								while (true)
								{
									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now == Token{ TokenType::Operator, ";" }) break;
									init_expr.push_back(now);
								}
								while (true)
								{
									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now == Token{ TokenType::Operator, ";" }) break;
									cnd_expr.push_back(now);
								}
								while (true)
								{
									if (next_or_exit(tokens)) throw std::exception("Unexcepted Syntax");
									if (now == Token{ TokenType::Operator, ")" }) break;
									incr_expr.push_back(now);
								}

								stack.push(&keyword.push_back({ TokenType::Structure, "for-body" }));
								statement.push("while");
							}
							else if (now == Token{ TokenType::Operator, "{" }) {
								if (is_statement(statement.top()))
									statement.top() += "{}";
								else
								{
									stack.push(&stack.top()->push_back(Token{ TokenType::Structure, "{}" }));
									statement.push("{}");
								}
							}
							else if (now == Token{ TokenType::Operator, "}" }) {
								if (statement.size() && is_extended_statement(statement.top()))
								{
									stack.pop();
									statement.pop();
								}
								stack.pop();
								statement.pop();
							}
							else if (now == Token{ TokenType::Operator, ";" }) {
								if (statement.size() && is_statement(statement.top()))
								{
									stack.pop();
									statement.pop();
								}
							}
							else {
								stack.top()->push_back(now);
							}
						}
					}
				}
				if (next_or_exit(tokens)) goto exit;
			};
		}
		catch (std::exception)
		{
			std::cout << "Error!" << '\n';
		}
	exit:;
	{
		debug_tree::TreeAnalyzeData* tad = new debug_tree::TreeAnalyzeData();
		size_t y = 0;
		logic.analyze(tad, 0, y);
		tad->Draw(std::cout);
		return;
	}
	}
private:
	Token now;
	std::list<Token>::const_iterator itr;
	bool next_or_exit(const std::list<Token>& tokens) {
		++itr;
		if (itr == tokens.cend())
			return true;
		now = (*itr);
		std::cout << now.str << " ";
		if (now.str == ";") std::cout << '\n';
		return false;
	}

	bool is_statement(const std::string& str)
	{
		return str == "do" || str == "if" || str == "else" || str == "var" || str == "while";
	}

	bool is_extended_statement(const std::string& str)
	{
		if ((*str.crbegin() + 1) == '{' && (*str.crbegin()) == '}')
			return is_statement(str.substr(str.size() - 2));
		else return false;
	}
};

class ScriptCompiler {
public:

	ScriptCompiler(const std::string& content)
	{
		Lexer lexer(content.cbegin(), content.cend());
		LogicBuilder logic_builder(lexer.tokens);
		return;
	}
};

void main()
{
	std::ifstream script_file("sample.js");
	if (!script_file.is_open())
		throw std::invalid_argument("Script file not exists.");
	std::string script_content((std::istreambuf_iterator<char>(script_file)), (std::istreambuf_iterator<char>()));
	ScriptCompiler sc(script_content);
}