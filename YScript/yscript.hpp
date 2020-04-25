#pragma once

#include <list>
#include <map>
#include <memory>
#include <functional>

#include "tree.hpp"
#include <Windows.h>

#ifdef _DEBUG
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

#ifndef __YSCRIPT__

namespace YScript
{
	typedef int32_t I32;
	typedef float Float;
	typedef std::string String;

	namespace YObjectType
	{
		static enum  YObjectType : uint64_t {
			Null = 1, Bool, I32, F32, String, Bytes,
			List, Dict, Tuple, Set,
			Built_In_Function, Function,
			Class, Instance, Method,
			Type
		};
	};

	static std::map<uint64_t, String> YObjectTypenames = {
		{YObjectType::Null, "Null"},
		{YObjectType::Bool, "Bool"},
		{YObjectType::I32, "I32"},
		{YObjectType::String, "String"},
		{YObjectType::Bytes, "Bytes"},
		{YObjectType::F32, "F32"},
		{YObjectType::List, "List"},
		{YObjectType::Dict, "Dict"},
		{YObjectType::Tuple, "Tuple"},
		{YObjectType::Set, "Set"},
		{YObjectType::Function, "Function"},
		{YObjectType::Class, "Class"},
	};

	static uint64_t yobject_incr = 0;
	struct YObject {
		const uint64_t type_id = 0;
		const uint64_t object_id = ++yobject_incr;
		void* const data = nullptr;
		std::vector<std::pair<String, std::shared_ptr<YObject>>> attrs;
		YObject(const uint64_t type, void* const data) : data(data), type_id(type) {
			char buffer[256];
			sprintf_s(buffer, "CREATE 0x%08X \t0x%p\n", object_id, this);
			OutputDebugStringA(buffer);
		}
		~YObject() {
			char buffer[256];
			sprintf_s(buffer, "DELETE 0x%08X \t0x%p\n", object_id, this);
			OutputDebugStringA(buffer);
			if (data != nullptr)
				delete data;
		}
	};

	typedef std::shared_ptr<YObject> YPtr;
	bool operator == (YPtr lhs, YPtr rhs);
	typedef std::vector<std::pair<YPtr, YPtr>> Dict;
	typedef std::vector<YPtr> Tuple;
	typedef std::vector<YPtr> Set;
	typedef std::vector<YPtr> List;

	const static std::list<std::string> __keywords__ = {
		"function",
		"for",
		"while",
		"do",
		"else",
		"if",
		"in",
		"var",
		"break",
		"continue",
		"null", "true", "false",
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
		Value,
		Literal
	};

	struct Token {
		TokenType type;
		std::string str;
	};
	bool operator == (const Token& left, const Token& right);
	bool operator != (const Token& left, const Token& right);

	struct GlobalBinding {
		std::map<uint64_t, std::map<std::string, YPtr>> value_map;
		YPtr const_true = std::make_shared<YObject>(YObjectType::Bool, new bool(true));
		YPtr const_false = std::make_shared<YObject>(YObjectType::Bool, new bool(false));
		YPtr const_null = std::make_shared<YObject>(YObjectType::Null, nullptr);

		YPtr literal_decode(const std::string& src);
		std::string __repr__(YPtr obj);
		YPtr casting(YPtr obj, const uint64_t target);
		YPtr deepcopy(const YPtr src);
		YPtr operand(const std::string& op, YPtr lhs, YPtr rhs);

		GlobalBinding();
	};
	typedef std::function<YPtr(GlobalBinding*, YPtr, YPtr)> BuiltInFunction;
	std::string literal_encode(const std::string& src);

	class Lexer
	{
	public:
		std::list<Token> tokens;
		Lexer(std::string::const_iterator begin, std::string::const_iterator end);
		bool static match(const std::string& token, std::string::const_iterator itr, std::string::const_iterator end);
		bool static is_space(char ch);
	private:
		LexingState lexing_state = LexingState::Logic;
		uint64_t passing_count = 0;
		std::string leftover = "";
		std::string string_builder = "";
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
				std::vector<std::string>{"!", "+ ", "-", "++", "--"},
				true, false, true
			},
			OperatorPrecedence{
				std::vector<std::string>{"**"},
				true, true, true
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
				std::vector<std::string>{">", ">=", "<", "<="},
				true, true, true
			},
			OperatorPrecedence{
				std::vector<std::string>{"==", "!="},
				true, true, true
			},
		};

		void read_brackets(std::vector<Token> tokens, tree<Token>* root);
		tree<Token>* read_operators(tree<Token>* parent, tree<Token>* root, const char& end_token);
	};

	class Assembler {
	public:
		Assembler(const tree<Token>& logic);
		std::vector<std::string> bytecodes;
	private:
		GlobalBinding* global = nullptr;
		void process_expression(const tree<Token>* expr, uint64_t depth, uint64_t passing, uint64_t count);
	};

	class Executor {
	public:
		Executor(GlobalBinding* const  global, const std::vector<std::string>& bytecodes);
	private:
		std::stack<YPtr> stack;
		GlobalBinding* global = nullptr;
	};

	class ScriptEngine {
	public:
		ScriptEngine(const std::string& content)
		{
			Lexer lexer(content.cbegin(), content.cend());
			LogicBuilder logic_builder(lexer.tokens);
			Assembler assembler{ logic_builder.logic };
			Executor executor{ &global , assembler.bytecodes };
		}
	private:
		GlobalBinding global = {};
	};
};

#define __YSCRIPT__
#endif