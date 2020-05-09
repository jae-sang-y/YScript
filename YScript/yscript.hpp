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

	extern struct YObject;
	extern struct GlobalBinding;

	struct Attribute {
		String key;
		std::shared_ptr<YObject> value;
	};
	typedef std::vector<Attribute> Attributes;
	typedef std::shared_ptr<YObject> YPtr;
	typedef std::vector<std::pair<YPtr, YPtr>> Dict;
	typedef std::vector<YPtr> List;
	typedef std::vector<std::string> Code;
	typedef std::function<YPtr(GlobalBinding*, List&, Attributes&)> RawFunction;
	struct FunctionHeader {
		bool unlimit_args = false;
		bool unlimit_kwargs = false;
		uint64_t argc = 0;
		Attributes kwargs;
	};
	struct BuiltInFunction : public FunctionHeader {
		RawFunction function;
	};

	static uint64_t __yobject_incr__ = 0;
	struct YObject {
		const uint64_t object_id = ++__yobject_incr__;
		void* const data = nullptr;
		bool is_const = false;
		String debug_comment = "";

		YObject(std::shared_ptr<YObject> type, void* const data) : data(data), type(type) {
			if (type == nullptr)
				debug_comment = "unknown";
			else
				debug_comment = type->AsStr();
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

		bool CompareType(String&& name)
		{
			if (type == nullptr) throw "Exception";
			return (*(String*)type->data) == name;
		}
		String& AsStr()
		{
			if (!(CompareType("str") || CompareType("type") || CompareType("object") || CompareType("exception"))) throw "Exception";
			return *(String*)data;
		}
		bool& AsBool()
		{
			if (!(CompareType("bool"))) throw "Exception";
			return *(bool*)data;
		}
		I32& AsI32()
		{
			if (!(CompareType("i32"))) throw "Exception";
			return *(I32*)data;
		}
		F32& AsF32()
		{
			if (!(CompareType("f32"))) throw "Exception";
			return *(F32*)data;
		}
		List& AsList()
		{
			if (!(CompareType("list"))) throw "Exception";
			return *(List*)data;
		}
		Dict& AsDict()
		{
			if (!(CompareType("dict"))) throw "Exception";
			return *(Dict*)data;
		}
		Attributes& AsAttributes()
		{
			if (!(CompareType("11"))) throw "Exception";
			return *(Attributes*)data;
		}
		BuiltInFunction& AsBuiltInFunction()
		{
			if (!(CompareType("built_in_function"))) throw "Exception";
			return *(BuiltInFunction*)data;
		}
		void SetType(std::shared_ptr<YObject> type)
		{
			this->type = type;
		}
		std::shared_ptr<YObject> GetType()
		{
			return type;
		}
	private:
		Attributes attrs;
		std::shared_ptr<YObject> type = nullptr;
	};

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

	struct GlobalBinding {
		YPtr type_type = nullptr;

		YPtr type_null = nullptr;
		YPtr type_bool = nullptr;
		YPtr type_exception = nullptr;

		YPtr type_i32 = nullptr;
		YPtr type_f32 = nullptr;
		YPtr type_str = nullptr;

		YPtr type_built_in_function = nullptr;
		YPtr type_function = nullptr;

		YPtr type_list = nullptr;
		YPtr type_dict = nullptr;
		YPtr type_object = nullptr;

		YPtr const_true = nullptr;
		YPtr const_false = nullptr;
		YPtr const_null = nullptr;
		YPtr const_empty_function = nullptr;

		std::map<uint64_t, std::map<std::string, YPtr>> global_value_map;
		std::map<std::string, std::map<uint64_t, std::map<std::string, YPtr>>> local_value_map;
		YPtr literal_decode(const std::string& src);
		std::string string_decode(const std::string& src);

		YPtr CreateType(String&& str)
		{
			return CreateYPtr(type_type, new String(str));
		}

		YPtr CreateNull()
		{
			return CreateYPtr(type_null, nullptr);
		}

		YPtr CreateBool(bool&& data = bool())
		{
			return CreateYPtr(type_bool, new bool(data));
		}
		YPtr CreateI32(I32&& data = I32())
		{
			return CreateYPtr(type_i32, new I32(data));
		}
		YPtr CreateF32(F32&& data = F32())
		{
			return CreateYPtr(type_f32, new F32(data));
		}
		YPtr CreateStr(String&& data = String())
		{
			return CreateYPtr(type_str, new String(data));
		}
		YPtr CreateException(String&& data = String())
		{
			return CreateYPtr(type_exception, new String(data));
		}

		YPtr CreateBuiltInFunction(BuiltInFunction&& data)
		{
			return CreateYPtr(type_built_in_function, new BuiltInFunction(data));
		}

		YPtr CreateBuiltInFunction(FunctionHeader&& header, RawFunction&& raw_func) {
			auto* data = new BuiltInFunction();
			data->argc = header.argc;
			data->kwargs = header.kwargs;
			data->unlimit_args = header.unlimit_args;
			data->unlimit_kwargs = header.unlimit_kwargs;
			data->function = raw_func;
			return CreateYPtr(type_built_in_function, data);
		}

		YPtr CreateList(List&& data = List())
		{
			return CreateYPtr(type_list, new List(data));
		}
		YPtr CreateDict(Dict&& data = Dict())
		{
			return CreateYPtr(type_dict, new Dict(data));
		}
		YPtr CreateObject(String&& data = String())
		{
			return CreateYPtr(type_object, new String(data));
		}

		YPtr CallFunction(YPtr func, List& args, Attributes& kwargs)
		{
			if (func->CompareType("built_in_function"))
			{
				BuiltInFunction bf = func->AsBuiltInFunction();
				if (!bf.unlimit_args)
				{
					if (args.size() != bf.argc)
						throw "Argument(s) size isn't match";
				}
				if (!bf.unlimit_kwargs)
				{
					for (auto kwarg : kwargs)
					{
						bool is_match = false;
						for (auto expect_kwarg : bf.kwargs)
						{
							if (expect_kwarg.key == kwarg.key)
							{
								is_match = true;
								break;
							}
						}
						if (!is_match)
							throw "Unknown Keword argument(s) exist";
					}
				}
				return bf.function(this, args, kwargs);
			}
			else throw "Exception";
		}

		bool CompareType(YPtr lhs, YPtr rhs)
		{
			return lhs->CompareType(String(rhs->GetType()->AsStr()));
		}

		YPtr RunOperator(const String&& op_name, YPtr self)
		{
			auto func = self->GetType()->get_attr(op_name);
			auto args = List();
			auto kwargs = Attributes();

			args.push_back(self);
			return CallFunction(func, args, kwargs);
		}

		YPtr RunOperator(const String&& op_name, YPtr self, YPtr other)
		{
			auto func = self->GetType()->get_attr(op_name);
			auto args = List();
			auto kwargs = Attributes();

			args.push_back(self);
			args.push_back(other);
			return CallFunction(func, args, kwargs);
		}

		GlobalBinding();

	private:
		void assign_built_in(const std::string value_name, YPtr object);
		YPtr CreateYPtr(YPtr type, void* value)
		{
			return std::make_shared<YObject>(type, value);
		}

		void BuildType(YPtr& type);
		void BuildBool(YPtr& type);
		void BuildNull(YPtr& type);
		void BuildI32(YPtr& type);
		void BuildF32(YPtr& type);
		void BuildStr(YPtr& type);
		void BuildException(YPtr& type);
		void BuildList(YPtr& type);
		void BuildDict(YPtr& type);
		void BuildObject(YPtr& type);
		void BuildFunctions();
	};
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
			Executor executor{ &global, "file" , assembler.bytecodes };
		}
	private:
		GlobalBinding global = {};
	};
};

#define __YSCRIPT__
#endif