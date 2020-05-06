#include "yscript.hpp"

namespace YScript
{
	YPtr GlobalBinding::call_built_in_method(void* func, YPtr self, YPtr args, YPtr kwargs)
	{
		return (*(BuiltInMethod*)func)(this, self, args, kwargs);
	}

	YPtr GlobalBinding::call_built_in_function(void* func, YPtr args, YPtr kwargs)
	{
		return (*(BuiltInFunction*)func)(this, args, kwargs);
	}

	namespace Built_In {
		static std::unordered_map<String, BuiltInMethod> _I32 = {
			{"__gt__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_i32)
					if (*(I32*)self->data > * (I32*)other->data) return global->const_true;
					else return global->const_false;
				else throw std::exception("no");
			}},
			{ "__ge__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_i32)
					if (*(I32*)self->data >= *(I32*)other->data) return global->const_true;
					else return global->const_false;
				else throw std::exception("no");
			}},
			{"__le__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_i32)
					if (*(I32*)self->data <= *(I32*)other->data) return global->const_true;
					else return global->const_false;
				else throw std::exception("no");
			}},
			{ "__lt__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_i32)
					if (*(I32*)self->data < *(I32*)other->data)
						return global->const_true;
					else
						return global->const_false;
				else throw std::exception("no");
			}},

			{ "__eq__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_i32)
					if (*(I32*)self->data == *(I32*)other->data) return global->const_true;
					else return global->const_false;
				else throw std::exception("no");
			}},
			{ "__ne__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_i32)
					if (*(I32*)self->data != *(I32*)other->data) return global->const_true;
					else return global->const_false;
				else throw std::exception("no");
			}},

			{ "__add__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_i32)
				{
					auto obj = global->call_built_in_function(global->type_i32->get_attr("__new__")->data, global->const_null, global->const_null);
					*(I32*)obj->data = *(I32*)self->data + *(I32*)other->data;
					return obj;
				}
				else throw std::exception("no");
			}},

			{"__repr__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				return global->new_str(std::to_string(*(I32*)self->data));
			}}
		};

		static std::unordered_map<String, BuiltInMethod> _BOOL = {
			{"__init__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto data = list->at(0);

				if (CompareType(data->type, global->type_bool))
					*(bool*)self->data = *(bool*)data->data;
				else if (CompareType(data->type, global->type_null))
					*(bool*)self->data = false;
				else *(bool*)self->data = true;
				return global->const_null;
			}},
			{"__repr__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				if (*(bool*)self->data)
					return global->new_str("true");
				else
					return global->new_str("false");
			}}
		};

		static std::unordered_map<String, BuiltInMethod> _NULL = {
			{"__repr__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				return global->new_str("null");
			}}
		};

		static std::unordered_map<String, BuiltInMethod> _F32 = {};

		static std::unordered_map<String, BuiltInMethod> _STR = {
			{"__init__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				if (list->size())
				{
				}
				return global->const_null;
			}},
			{ "__eq__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_str)
					if (*(String*)self->data == *(String*)other->data) return global->const_true;
					else return global->const_false;
				else throw std::exception("no");
			}},

			{"__add__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto other = list->at(0);
				if (other->type == global->type_str)
				{
					*(String*)self->data = *(String*)self->data + *(String*)other->data;
				}
				else
				{
					auto result = global->call_built_in_method(other->get_attr("__repr__")->data, other, global->new_list(List()), global->new_dict(Dict()));
					if (result->type != global->type_str) throw std::exception("no");
					*(String*)self->data = *(String*)self->data + *(String*)result->data;
				}
				return self;
			}},

			{"__repr__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				return global->new_str(string_encode(*(String*)self->data));
			}},
		};

		static std::unordered_map<String, BuiltInMethod> _LIST = {
			{"__init__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				if (list->size())
				{
				}
				return global->const_null;
			}},
			{"__getitem__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				auto data = GetList(args)->at(0);

				List* list = GetList(self);
				if (!CompareType(data->type, global->type_i32))
					throw std::exception("1234");
				I32 index = *(I32*)data->data;
				if (index < 0) index += list->size();

				if (index >= list->size()) throw std::exception("Key Error");
				return list->at(index);
			}},
			{ "__len__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				return global->new_i32(GetList(self)->size());
			}},
			{"__repr__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				YPtr str = global->new_str("[");
				uint64_t k = 0;
				for (auto element : *(List*)self->data)
				{
					if (k > 0)
					{
						global->call_built_in_method(str->get_attr("__add__")->data,
							str, CreateObject(global->type_list, new List{ global->new_str(", ") }), global->const_null);
					}
					global->call_built_in_method(str->get_attr("__add__")->data,
						str, CreateObject(global->type_list, new List{
							global->call_built_in_method(element->get_attr("__repr__")->data,
						element, CreateObject(global->type_list, new List{ element }), global->const_null)
							}), global->const_null);
					++k;
				}
				global->call_built_in_method(str->get_attr("__add__")->data,
					str, CreateObject(global->type_list, new List{ global->new_str("]") }), global->const_null);
				return str;
			}},
		};

		static std::unordered_map<String, BuiltInMethod> _DICT = {
			{"__init__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				if (list->size())
				{
					auto data = list->at(0);

					if (CompareType(data->type, global->type_dict))
						*(bool*)self->data = *(bool*)data->data;
					else throw std::exception("Unknown type");
				}
				return global->const_null;
			}},
			{"__getitem__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				List* list = GetList(args);
				auto data = list->at(0);

				Dict* dict = GetDict(self);
				for (auto& pair : *dict)
				{
					if (CompareType(pair.first->type, data->type))
					{
						auto result = global->call_built_in_method(pair.first->get_attr("__eq__")->data,
							pair.first, global->new_list(List{ data }), global->const_null);
						if (!CompareType(result->type, global->type_bool))
							throw std::exception("Expect bool type");
						if (*(bool*)result->data)
							return pair.second;
					}
				}
				throw std::exception("Key Error");
			}},
			{"__repr__" , [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				YPtr str = global->new_str("{");
				uint64_t k = 0;
				for (auto pair : *(Dict*)self->data)
				{
					if (k > 0)
					{
						global->call_built_in_method(str->get_attr("__add__")->data,
							str, CreateObject(global->type_list, new List{ global->new_str(", ") }), global->const_null);
					}
					global->call_built_in_method(str->get_attr("__add__")->data,
						str, CreateObject(global->type_list, new List{
							global->call_built_in_method(pair.first->get_attr("__repr__")->data,
						pair.first, CreateObject(global->type_list, new List{ pair.first }), global->const_null)
							}), global->const_null);

					global->call_built_in_method(str->get_attr("__add__")->data,
						str, CreateObject(global->type_list, new List{ global->new_str(": ") }), global->const_null);

					global->call_built_in_method(str->get_attr("__add__")->data,
						str, CreateObject(global->type_list, new List{
							global->call_built_in_method(pair.second->get_attr("__repr__")->data,
						pair.second, CreateObject(global->type_list, new List{ pair.second }), global->const_null)
							}), global->const_null);
					++k;
				}
				global->call_built_in_method(str->get_attr("__add__")->data,
					str, CreateObject(global->type_list, new List{ global->new_str("}") }), global->const_null);
				return str;
			}},
		};

		static std::unordered_map<String, BuiltInMethod> _TYPE = {
			{"__repr__", [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
				return global->new_str("<type '" + *(String*)self->data + "'>");
			}},
			{"__type__",  [](GlobalBinding* global, YPtr self, YPtr args, YPtr kwargs) {
			return self->type;
			}},
		};
	}

	void GlobalBinding::assign_built_in(const std::string value_name, YPtr object)
	{
		value_map[value_name.size()][value_name] = object;
	}

	YPtr GlobalBinding::new_dict(Dict dict)
	{
		auto result = call_built_in_function(type_dict->get_attr("__new__")->data, const_null, const_null);
		*(Dict*)result->data = dict;
		return result;
	}

	YPtr GlobalBinding::new_list(List list)
	{
		auto result = call_built_in_function(type_list->get_attr("__new__")->data, const_null, const_null);
		*(List*)result->data = list;
		return result;
	}

	YPtr GlobalBinding::new_str(String str)
	{
		auto result = call_built_in_function(type_str->get_attr("__new__")->data, const_null, const_null);
		*(String*)result->data = str;
		return result;
	}

	YPtr GlobalBinding::new_i32(I32 i32)
	{
		auto result = call_built_in_function(type_i32->get_attr("__new__")->data, const_null, const_null);
		*(I32*)result->data = i32;
		return result;
	}

	YPtr GlobalBinding::new_f32(F32 f32)
	{
		auto result = call_built_in_function(type_f32->get_attr("__new__")->data, const_null, const_null);
		*(F32*)result->data = f32;
		return result;
	}

	GlobalBinding::GlobalBinding() {
		built_in_types = {
			type_type, type_i32, type_f32, type_str, type_null, type_bool,
			type_built_in_function, type_built_in_method, type_function, type_method,
			type_list, type_dict
		};

		for (auto method : Built_In::_TYPE)
			type_type->assign_attr(method.first, CreateObject(type_built_in_method, new BuiltInMethod(method.second)));

		for (auto type : built_in_types)
		{
			type->type = type_type;
			type->type_name = "type";
			type_type->inherit_to_subclass(type);
		}

		for (auto method : Built_In::_BOOL) type_bool->overload_attr(method.first, CreateObject(type_built_in_method, new BuiltInMethod(method.second)));
		for (auto method : Built_In::_NULL) type_null->overload_attr(method.first, CreateObject(type_built_in_method, new BuiltInMethod(method.second)));
		for (auto method : Built_In::_I32) type_i32->overload_attr(method.first, CreateObject(type_built_in_method, new BuiltInMethod(method.second)));
		for (auto method : Built_In::_F32) type_f32->overload_attr(method.first, CreateObject(type_built_in_method, new BuiltInMethod(method.second)));
		for (auto method : Built_In::_STR) type_str->overload_attr(method.first, CreateObject(type_built_in_method, new BuiltInMethod(method.second)));
		for (auto method : Built_In::_LIST) type_list->overload_attr(method.first, CreateObject(type_built_in_method, new BuiltInMethod(method.second)));
		for (auto method : Built_In::_DICT) type_dict->overload_attr(method.first, CreateObject(type_built_in_method, new BuiltInMethod(method.second)));

		type_built_in_method->overload_attr("__repr__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			return global->new_str("<built-in method>");
			})));

		type_built_in_function->overload_attr("__repr__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			return global->new_str("<built-in function>");
			})));

		type_type->assign_attr("__new__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			auto obj = CreateObject(global->type_type, new String{ "unknown" });
			global->type_type->inherit_to_subclass(obj);
			return obj;
			})));

		type_bool->assign_attr("__new__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			auto obj = CreateObject(global->type_bool, new bool{ false });
			global->type_bool->inherit_to_subclass(obj);
			return obj;
			})));

		type_null->assign_attr("__new__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			auto obj = CreateObject(global->type_null, nullptr);
			global->type_null->inherit_to_subclass(obj);
			return obj;
			})));

		type_i32->assign_attr("__new__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			auto obj = CreateObject(global->type_i32, new I32{ 0 });
			global->type_i32->inherit_to_subclass(obj);
			return obj;
			})));

		type_f32->assign_attr("__new__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			auto obj = CreateObject(global->type_f32, new F32());
			global->type_f32->inherit_to_subclass(obj);
			return obj;
			})));

		type_str->assign_attr("__new__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			auto obj = CreateObject(global->type_str, new String());
			global->type_str->inherit_to_subclass(obj);
			return obj;
			})));

		type_list->assign_attr("__new__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			auto obj = CreateObject(global->type_list, new List());
			global->type_list->inherit_to_subclass(obj);
			return obj;
			})));

		type_dict->assign_attr("__new__", CreateObject(type_built_in_function, new BuiltInFunction([](GlobalBinding* global, YPtr args, YPtr kwargs) {
			auto obj = CreateObject(global->type_dict, new Dict());
			global->type_dict->inherit_to_subclass(obj);
			return obj;
			})));

		assign_built_in("type", type_type);
		assign_built_in("bool", type_bool);
		assign_built_in("null", type_null);
		assign_built_in("i32", type_i32);
		assign_built_in("f32", type_f32);
		assign_built_in("str", type_str);
		assign_built_in("list", type_list);
		assign_built_in("dict", type_dict);
		assign_built_in("built_in_function", type_built_in_function);
		assign_built_in("built_in_method", type_built_in_method);

		const_true = CreateObject(type_bool, new bool{ true });
		const_false = CreateObject(type_bool, new bool{ false });
		const_null = CreateObject(type_null, nullptr);
		built_in_consts = {
		   const_true, const_false, const_null
		};
		for (auto a_const : built_in_consts)
		{
			a_const->type_name = *(String*)a_const->type->data;
			a_const->type->inherit_to_subclass(a_const);
			a_const->is_const = true;
		}

		assign_built_in("print", CreateObject(type_built_in_function, NEW BuiltInFunction(print)));

		//auto func = (*(BuiltInMethod*)type_i32->get_attr("__repr__")->data);
		//func(this, type_i32, const_null, const_null);
		//{
			//value_map[4]["test"] = CreateObject(YObjectType::Instance, nullptr);
//			value_map[4]["test"]->attrs.push_back(std::pair(String("a"), CreateObject(YObjectType::I32, new I32(1234))));
		//}
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
			return new_str(string_decode(src));
		else
		{
			if (src.find('.') == -1)
				return new_i32(std::stol(src));
			else
				return new_f32(std::stof(src));
		}
	}
}