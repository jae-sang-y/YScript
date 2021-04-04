#include "yscript.hpp"

namespace YScript
{
	void GlobalBinding::assign_built_in(const std::string value_name, YPtr object)
	{
		object->debug_comment += " <" + value_name + ">";
		global_value_map[value_name.size()][value_name] = object;
	}

	void GlobalBinding::BuildType(YPtr& type) {
		type->assign_attr(
			"__repr__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					   auto self = args.at(0);
					   if (!self->CompareType("type"))
						   throw "Exception";
					   return global->CreateStr("<type '" + self->AsStr() + "'>");
				} }
		));
	}
	void GlobalBinding::BuildNull(YPtr& type) {
		type->assign_attr(
			"__repr__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					return global->CreateStr("null");
				} }
		));
	}
	void GlobalBinding::BuildBool(YPtr& type) {
		type->assign_attr(
			"__repr__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					if (self->AsBool())
						return global->CreateStr("true");
					else
						return global->CreateStr("false");
				} }
		));
	}
	void GlobalBinding::BuildI32(YPtr& type) {
		type->assign_attr(
			"__repr__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					return global->CreateStr(std::to_string(self->AsI32()));
				} }
		));

		type->assign_attr(
			"__add__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("i32"))
						return global->CreateI32(self->AsI32() + other->AsI32());
					else
						return global->CreateStr(global->RunOperator("__repr__", self)->AsStr() + global->RunOperator("__repr__", other)->AsStr());
				} }
		));

		type->assign_attr(
			"__sub__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("i32"))
						return global->CreateI32(self->AsI32() - other->AsI32());
					else
						return global->CreateException("i32: Can't __sub__ by " + other->GetType()->AsStr());
				} }
		));

		type->assign_attr(
			"__gt__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("i32"))
						return global->CreateBool(self->AsI32() > other->AsI32());
					else
						return global->CreateException("i32: Can't __gt__ by " + other->GetType()->AsStr());
				} }
		));
		type->assign_attr(
			"__ge__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("i32"))
						return global->CreateBool(self->AsI32() >= other->AsI32());
					else
						return global->CreateException("i32: Can't __ge__ by " + other->GetType()->AsStr());
				} }
		));
		type->assign_attr(
			"__le__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("i32"))
						return global->CreateBool(self->AsI32() <= other->AsI32());
					else
						return global->CreateException("i32: Can't __le__ by " + other->GetType()->AsStr());
				} }
		));
		type->assign_attr(
			"__lt__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("i32"))
						return global->CreateBool(self->AsI32() < other->AsI32());
					else
						return global->CreateException("i32: Can't __lt__ by " + other->GetType()->AsStr());
				} }
		));

		type->assign_attr(
			"__eq__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("i32"))
						return global->CreateBool(self->AsI32() == other->AsI32());
					else
						return global->CreateException("i32: Can't __eq__ by " + other->GetType()->AsStr());
				} }
		));
		type->assign_attr(
			"__ne__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("i32"))
						return global->CreateBool(self->AsI32() != other->AsI32());
					else
						return global->CreateException("i32: Can't __ne__ by " + other->GetType()->AsStr());
				} }
		));
	}
	void GlobalBinding::BuildF32(YPtr& type) {
	}
	void GlobalBinding::BuildException(YPtr& type) {
		type->assign_attr(
			"__repr__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					return global->CreateStr("<exception " + String(self->AsStr()) + ">");
				} }
		));
	}
	void GlobalBinding::BuildStr(YPtr& type) {
		type->assign_attr(
			"__add__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("str"))
						return global->CreateStr(self->AsStr() + other->AsStr());
					else
						return global->CreateStr(self->AsStr() + global->RunOperator("__repr__", other)->AsStr());
				} }
		));
		type->assign_attr(
			"__repr__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					return global->CreateStr(String(self->AsStr()));
				} }
		));
		type->assign_attr(
			"__eq__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					YPtr self = args.at(0);
					YPtr other = args.at(1);

					if (other->CompareType("str"))
						return global->CreateBool(self->AsStr() == other->AsStr());
					else
						return global->CreateBool(false);
				} }
		));
	}
	void GlobalBinding::BuildList(YPtr& type) {
		type->assign_attr(
			"__getitem__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					   YPtr self = args.at(0);
					   List keys = args.at(1)->AsList();

					   I32 k = 0;
					   I32 t = keys.at(0)->AsI32();
					   if (t < 0) t += self->AsList().size();
					   for (auto& item : self->AsList())
					   {
						   if (k == t)
								return item;
						   ++k;
					   }
					   throw "exception";
				} }
		));
		type->assign_attr(
			"__len__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					   YPtr self = args.at(0);

					   return global->CreateI32(self->AsList().size());
				} }
		));
		type->assign_attr(
			"__repr__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					   YPtr self = args.at(0);

					   YPtr obj = global->CreateStr("[");
					   String& str = obj->AsStr();
					   uint64_t k = 0;
					   for (YPtr item : self->AsList())
					   {
						   if (k > 0) str += ", ";
						   str += global->RunOperator("__repr__", item)->AsStr();
						   ++k;
					   }
					   str += "]";
					   return obj;
				} }
		));
	}
	void GlobalBinding::BuildDict(YPtr& type) {
		type->assign_attr(
			"__getitem__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 2, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					   YPtr self = args.at(0);
					   YPtr key = args.at(1);

					   for (auto& pair : self->AsDict())
					   {
						   if (global->CompareType(pair.first, key))
						   {
							   if (global->RunOperator("__eq__", pair.first, key)->AsBool())
								   return pair.second;
						   }
					   }
					   throw "exception";
				} }
		));
		type->assign_attr(
			"__repr__", CreateBuiltInFunction(
				FunctionHeader{
					false, false, 1, Attributes()
				},
				RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
					   YPtr self = args.at(0);

					   YPtr obj = global->CreateStr("{");
					   String& str = obj->AsStr();
					   uint64_t k = 0;
					   for (auto pair : self->AsDict())
					   {
						   if (k > 0) str += ", ";
						   str += global->RunOperator("__repr__", pair.first)->AsStr();
						   str += ": ";
						   str += global->RunOperator("__repr__", pair.second)->AsStr();

						   ++k;
					   }
					   str += "}";
					   return obj;
				} }
		));
	}
	void GlobalBinding::BuildObject(YPtr& type) {
	}
	void GlobalBinding::BuildFunctions() {
		assign_built_in("print", CreateBuiltInFunction(
			FunctionHeader{
				true, false, 0, Attributes()
			},
			RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
				for (YPtr arg : args)
				{
					if (arg->CompareType("built_in_function"))
					{
						std::cout << "<built-in method>";
					}
					else
					{
						YPtr func = arg->GetType()->get_attr("__repr__");
						auto args = List();
						auto kwargs = Attributes();

						args.push_back(arg);

						std::cout << global->CallFunction(func, args, kwargs)->AsStr() << " ";
					}
				}
				std::cout << "\r\n";
				return global->const_null;
			} }));

		assign_built_in("type", CreateBuiltInFunction(
			FunctionHeader{
				false, false, 1, Attributes()
			},
			RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
				YPtr self = args.at(0);
				return self->GetType();
			} }));

		assign_built_in("len", CreateBuiltInFunction(
			FunctionHeader{
				false, false, 1, Attributes()
			},
			RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
				YPtr self = args.at(0);
				return global->RunOperator("__len__", self);
			} }));
	}

	GlobalBinding::GlobalBinding() {
		type_type = std::make_shared<YObject>(nullptr, new  String("type"));
		type_type->SetType(this->type_type);
		type_type->debug_comment = "type";
		type_i32 = CreateType("i32");
		type_f32 = CreateType("f32");
		type_str = CreateType("str");
		type_null = CreateType("null");
		type_bool = CreateType("bool");
		type_exception = CreateType("exception");
		type_built_in_function = CreateType("built_in_function");
		type_function = CreateType("function");
		type_list = CreateType("list");
		type_dict = CreateType("dict");
		type_object = CreateType("object");

		assign_built_in("i32", type_i32);
		assign_built_in("f32", type_f32);
		assign_built_in("str", type_str);
		assign_built_in("bool", type_bool);
		assign_built_in("exception", type_exception);
		assign_built_in("func", type_function);
		assign_built_in("built_in_function", type_built_in_function);
		assign_built_in("list", type_list);
		assign_built_in("dict", type_dict);
		assign_built_in("object", type_object);

		const_true = CreateBool(true);
		const_false = CreateBool(false);
		const_null = CreateNull();
		const_empty_function = CreateBuiltInFunction(
			FunctionHeader{
				true, true, 0, Attributes()
			},
			RawFunction{ [](GlobalBinding* global, List& args, Attributes& kawrgs) {
				return global->const_null;
			} }
		);

		std::vector<YPtr> built_in_consts = {
		   const_true, const_false, const_null
		};

		for (auto a_const : built_in_consts)
			a_const->is_const = true;

		BuildType(type_type);
		BuildNull(type_null);
		BuildBool(type_bool);
		BuildBool(type_exception);
		BuildI32(type_i32);
		BuildF32(type_f32);
		BuildStr(type_str);
		BuildList(type_list);
		BuildDict(type_dict);
		BuildObject(type_object);
		BuildFunctions();
	}

	std::string string_encode(const std::string& src)
	{
		std::string a_return = "";
		uint64_t k = 0;
		a_return.push_back('\'');
		for (const char& ch : src)
		{
			for (const auto& pair : __string_literal_pairs__)
			{
				if (ch == pair.second)
				{
					a_return += pair.first;
					goto outer_continue;
				}
			}
			a_return.push_back(ch);
		outer_continue:;
			++k;
		}
		a_return.push_back('\'');
		return a_return;
	}

	std::string literal_encode(const std::string& src)
	{
		if (src.size() > 2 && *src.begin() == '"' && *src.rbegin() == '"')
			return string_encode(src.substr(1, src.size() - 2));
		else return src;
	}

	std::string GlobalBinding::string_decode(const std::string& src)
	{
		String str = "";
		uint64_t k = 0;
		uint64_t passing_count = 1;
		for (std::string::const_iterator csr = src.cbegin(); csr != src.cend(); ++csr, ++k)
		{
			if (k == src.size() - 1) continue;
			if (passing_count) { --passing_count; continue; }
			for (const auto& pair : __string_literal_pairs__)
			{
				if (Lexer::match(pair.first, csr, src.cend()))
				{
					str.push_back(pair.second);
					passing_count = pair.first.size() - 1;
					goto outer_continue;
				}
			}
			str.push_back(*csr);
		outer_continue:;
		}
		return str;
	}

	YPtr GlobalBinding::literal_decode(const std::string& src)
	{
		if (src == "true")
		{
			return const_true;
		}
		else if (src == "false")
		{
			return const_false;
		}
		else if (src == "null")
		{
			return const_null;
		}
		else if (src.at(0) == '\'')
			return CreateStr(string_decode(src));
		else
		{
			if (src.find('.') == -1)
				return CreateI32(std::stol(src));
			else
				return CreateF32(std::stof(src));
		}
	}
}