#pragma once

#include <list>
#include <map>
#include <memory>
#include <functional>

#include "tree.hpp"

#ifdef _DEBUG
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

#define DEBUG_YOBJECT

#ifndef __YSCRIPT__

namespace YScript
{
	typedef int32_t I32;
	typedef float F32;
	typedef std::string String;

	static uint64_t yobject_incr = 0;
	struct YObject {
		std::shared_ptr<YObject> type = nullptr;
		const uint64_t object_id = ++yobject_incr;
		void* const data = nullptr;
		bool is_const = false;
		String type_name = "";

		struct Attribute {
			String key;
			std::shared_ptr<YObject> value;
		};

		YObject(std::shared_ptr<YObject> type, void* const data) : data(data), type(type) {
			if (type == nullptr)
				type_name = "unknown";
			else
				type_name = *(String*)type->data;
		}
		~YObject() {
			if (data != nullptr)
				delete data;
		}
		std::shared_ptr<YObject> get_attr(String target)
		{
			for (auto attr : attrs)
				if (attr.key == target)
					return attr.value;
			throw std::exception((*(String*)type->data + " has no attribute: " + target).c_str());
		}
		void set_attr(String target, std::shared_ptr<YObject> value)
		{
			for (auto& attr : attrs)
				if (attr.key == target)
				{
					attr.value = value;
					return;
				}
			throw std::exception((*(String*)type->data + " has no attribute: " + target).c_str());
		}

		void assign_attr(String target, std::shared_ptr<YObject> value)
		{
			for (auto& attr : attrs)
				if (attr.key == target)
					throw std::exception((*(String*)type->data + " already has attribute: " + target).c_str());
			attrs.push_back(Attribute{ target, value });
		}

		void inherit_to_subclass(std::shared_ptr<YObject> subclass)
		{
			for (auto attr : attrs)
				subclass->overload_attr(attr.key, attr.value);
		}

		void overload_attr(String target, std::shared_ptr<YObject> value)
		{
			for (auto& attr : attrs)
				if (attr.key == target)
				{
					attr.value = value;
					return;
				}
			attrs.push_back(Attribute{ target, value });
		}

	private:
		std::vector<Attribute> attrs;
	};

	typedef std::shared_ptr<YObject> YPtr;
	typedef std::vector<std::pair<YPtr, YPtr>> Dict;
	typedef std::vector<YPtr> List;
	typedef std::vector<std::string> Code;

	static YPtr CreateObject(YPtr type, void* value)
	{
		return std::make_shared<YObject>(type, value);
	}

	const static std::list<std::string> __keywords__ = {
		"function",
		"for", "while", "do", "else", "if", "in",
		"break", "continue",
		"null", "true", "false",
	};

	const static std::list<std::string> __ops__ = {
		"++", "--",
		"==", "!=", ">=", "<=",
		"&&", "||",
		"+=", "-=", "*=", "/=", "%=",
		">", "<", "!", "=", "+", "-",
		"*", "/", "%", "[", "]", ".",
		"(", ")", ",", "?", ":", ";",
		"{", "}",
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

	const std::unordered_map<std::string, std::string> __operator_to_method__ = {
		{">", "__gt__"},
		{">=", "__ge__"},
		{"<", "__lt__"},
		{"<=", "__le__"},
		{"==", "__eq__"},
		{"!=", "__ne__"},

		{"+", "__add__"},
		{"-", "__sub__"},
		{"/", "__div__"},
		{"%", "__mod__"},
		{"*", "__mul__"},
		{"**", "__pow__"},
		{"&&", "__and__"},
		{"^^", "__xor__"},
		{"||", "__or__"},
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

	static bool CompareType(YPtr lhs, YPtr rhs)
	{
		return *(String*)lhs->data == *(String*)rhs->data;
	}

	static List* GetList(YPtr list) {
		if (*(String*)list->type->data != "list")
			throw std::exception("is not list");
		return (List*)list->data;
	}
	static Dict* GetDict(YPtr dict) {
		if (*(String*)dict->type->data != "dict")
			throw std::exception("is not dict");
		return (Dict*)dict->data;
	}

	struct GlobalBinding {
		YPtr type_type = CreateObject(nullptr, new String("type"));
		YPtr type_i32 = CreateObject(nullptr, new String("i32"));
		YPtr type_f32 = CreateObject(nullptr, new String("f32"));
		YPtr type_str = CreateObject(nullptr, new String("str"));
		YPtr type_null = CreateObject(nullptr, new String("null"));
		YPtr type_bool = CreateObject(nullptr, new String("bool"));

		YPtr type_built_in_function = CreateObject(nullptr, new String("built_in_function"));
		YPtr type_built_in_method = CreateObject(nullptr, new String("built_in_method"));
		YPtr type_function = CreateObject(nullptr, new String("user_defined_function"));
		YPtr type_method = CreateObject(nullptr, new String("method"));

		YPtr type_list = CreateObject(nullptr, new String("list"));
		YPtr type_dict = CreateObject(nullptr, new String("dict"));

		YPtr const_true = nullptr;
		YPtr const_false = nullptr;
		YPtr const_null = nullptr;

		std::list<YPtr> built_in_types;
		std::list<YPtr> built_in_consts;
		std::map<uint64_t, std::map<std::string, YPtr>> value_map;
		std::map<std::string, std::map<uint64_t, std::map<std::string, YPtr>>> local_value_map;
		YPtr call_built_in_function(void* func, YPtr args, YPtr kwargs);
		YPtr call_built_in_method(void* func, YPtr self, YPtr args, YPtr kwargs);
		YPtr literal_decode(const std::string& src);
		std::string string_decode(const std::string& src);

		YPtr new_list(List list);
		YPtr new_dict(Dict dict);
		YPtr new_str(String str);
		YPtr new_i32(I32 i32);
		YPtr new_f32(F32 f32);

		std::function<YPtr(GlobalBinding*, YPtr, YPtr)> print = [](GlobalBinding* global, YPtr args, YPtr kwargs) {
			for (auto arg : *(List*)args->data)
			{
				if (CompareType(arg->type, global->type_built_in_method))
				{
					std::cout << "<built-in method>";
					continue;
				}
				std::cout << *(String*)global->call_built_in_method(arg->type->get_attr("__repr__")->data, arg, global->const_null, global->const_null)->data << " ";
				//auto type = global->call_built_in_method(arg->get_attr("__type__")->data, arg, global->const_null, global->const_null);
				//std::cout << *(String*)global->call_built_in_method(type->get_attr("__repr__")->data, type, global->const_null, global->const_null)->data << " ";
			}
			std::cout << "\r\n";
			return global->const_null;
		};

		GlobalBinding();

	private:
		void assign_built_in(const std::string value_name, YPtr object);
	};
	typedef std::function<YPtr(GlobalBinding*, YPtr, YPtr)> BuiltInFunction;
	typedef std::function<YPtr(GlobalBinding*, YPtr, YPtr, YPtr)> BuiltInMethod;
	std::string string_encode(const std::string& src);
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
		Code bytecodes;
	private:
		GlobalBinding* global = nullptr;
		void process_expression(const tree<Token>* expr, uint64_t depth, uint64_t passing, uint64_t count);
	};

	class Executor {
	public:
		Executor(GlobalBinding* const  global, const std::string& namespace_name, const Code& bytecodes);
	private:
		std::map<uint64_t, std::map<std::string, YPtr>>* local_value_map;
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
			//try {
			Executor executor{ &global, "file" , assembler.bytecodes };
			//}
			//catch (std::exception e) {
//				e.what();
			//}
		}
	private:
		GlobalBinding global = {};
	};
};

#define __YSCRIPT__
#endif