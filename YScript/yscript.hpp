#pragma once

#include <list>

#include "tree.hpp"

#ifndef __YSCRIPT__

namespace YScript
{
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

	enum class LexingState {
		Logic,
		SingleQuotesString,
		DoubleQuotesString,
		CommentLine,
		CommentLines
	};

	enum class TokenType {
		Structure,
		Operator,
		Caculation,
		KeyWord,
		Value
	};

	struct Token {
		TokenType type;
		std::string str;
	};
	bool operator == (const Token& left, const Token& right);
	bool operator != (const Token& left, const Token& right);

	class Lexer
	{
	public:
		std::list<Token> tokens;
		Lexer(std::string::const_iterator begin, std::string::const_iterator end);
	private:
		LexingState lexing_state = LexingState::Logic;
		size_t passing_count = 0;
		std::string leftover = "";
		std::string string_builder = "";

		bool match(const std::string& token, std::string::const_iterator itr, std::string::const_iterator end);
		bool is_space(char ch);
	};

	class LogicBuilder {
	public:
		LogicBuilder(const std::list<Token>& tokens);
		tree<Token> logic{ Token{TokenType::Structure, "root"} };
	private:
		Token now;
		std::list<Token>::const_iterator itr;

		bool next_or_exit(const std::list<Token>& tokens);
	};

	class ExpressionEncoder {
	public:
		ExpressionEncoder(std::vector<Token> tokens, tree<YScript::Token>* target);
	private:
		struct OperatorPrecedence {
			std::vector<std::string> operators;
			bool left_to_right;
			bool left_op;
			bool right_op;
		};

		std::vector<OperatorPrecedence> op_precedences = {
			OperatorPrecedence{
				std::vector<std::string>{"++", "--"},
				true, false, true
			},
			OperatorPrecedence{
				std::vector<std::string>{"++", "--"},
				true, true, false
			},
			OperatorPrecedence{
				std::vector<std::string>{"*", "/", "%"},
				true, true, true
			},
			OperatorPrecedence{
				std::vector<std::string>{"+", "-"},
				true, true, true
			},
			OperatorPrecedence{
				std::vector<std::string>{"=", "+=", "-=", "*=", "/=", "%=", "!="},
				true, true, true
			},
		};

		void read_brackets(std::vector<Token> tokens, tree<Token>* root);
		tree<Token>* read_operators(tree<Token>* parent, tree<Token>* root, const char& end_token);
	};

	class Assembler {
	public:
		Assembler(const tree<Token>& logic);
	};

	class ScriptEngine {
	public:
		ScriptEngine(const std::string& content)
		{
			Lexer lexer(content.cbegin(), content.cend());
			LogicBuilder logic_builder(lexer.tokens);
			Assembler assembler{ logic_builder.logic };
		}
	};
}

#define __YSCRIPT__
#endif