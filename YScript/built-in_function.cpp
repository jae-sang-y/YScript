#include "yscript.hpp"

namespace YScript
{
	YPtr print(GlobalBinding* global, YPtr args, YPtr kwargs) {
		for (auto arg : *(List*)args->data)
			std::cout << global->__repr__(arg) << "\r\n";
		return global->const_null;
	}

	GlobalBinding::GlobalBinding() {
		value_map[5]["print"] = std::make_shared<YObject>(YObjectType::Built_In_Function, NEW BuiltInFunction(print));

		{
			value_map[4]["test"] = std::make_shared<YObject>(YObjectType::Instance, nullptr);
			value_map[4]["test"]->attrs.push_back(std::pair(String("a"), std::make_shared<YObject>(YObjectType::I32, new I32(1234))));
		}
	}

	std::string literal_encode(const std::string& src)
	{
		std::string a_return = "";
		if (src.size() > 2 && *src.begin() == '"' && *src.rbegin() == '"')
		{
			uint64_t k = 0;
			a_return.push_back('"');
			for (const char& ch : src)
			{
				if (k == 0 || k == src.size() - 1) {
					k += 1;
					continue;
				}
				for (const auto& pair : __string_literal_pairs__)
				{
					if (ch == pair.second)
					{
						a_return += pair.first;
						goto outer_continue;
					}
				}
				++k;
				a_return.push_back(ch);
			outer_continue:;
			}
			a_return.push_back('"');
		}
		else a_return = src;

		return a_return;
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
		else if (src.at(0) == '"')
		{
			String* str = NEW String();
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
						str->push_back(pair.second);
						goto outer_continue;
					}
				}
				str->push_back(*csr);
			outer_continue:;
			}
			return std::make_shared<YObject>(YObjectType::String, str);
		}
		else
		{
			if (src.find('.') == -1)
			{
				return std::make_shared<YObject>(YObjectType::I32, NEW I32(std::stof(src)));
			}
			else
			{
				return std::make_shared<YObject>(YObjectType::F32, NEW Float(std::stoi(src)));
			}
		}
	}

	std::string GlobalBinding::__repr__(YPtr obj)
	{
		switch (obj->type_id)
		{
		case YObjectType::I32: return "I32 " + std::to_string(*(I32*)obj->data);
		case YObjectType::F32: return "F32 " + std::to_string(*(Float*)obj->data);
		case YObjectType::Bool: return "Bool " + (*(bool*)obj->data) ? "true" : "false";
		case YObjectType::Null: return "null";
		case YObjectType::Function: return "Function 0x" + std::to_string(*(uint64_t*)obj->data);
		case YObjectType::String: return "\'" + literal_encode(*(std::string*)obj->data) + "\'";
		case YObjectType::List:
		{
			std::string result = "[";
			auto list = (List*)obj->data;
			for (size_t k = 0; k < list->size(); ++k)
			{
				if (k) result += ", ";
				result += __repr__((*list)[k]);
			}
			return result + "]";
		}
		case YObjectType::Dict:
		{
			std::string result = "{";
			auto dict = (Dict*)obj->data;
			for (size_t k = 0; k < dict->size(); ++k)
			{
				if (k) result += ", ";
				auto pair = (*dict)[k];
				result += __repr__(pair.first);
				result += ": ";
				result += __repr__(pair.second);
			}
			return result + "}";
		}
		}
		if (auto itr = YObjectTypenames.find(obj->type_id); itr != YObjectTypenames.end())
			return itr->second + " 0x" + std::to_string(obj->object_id);
		return "YObject 0x" + std::to_string(obj->object_id);
	}

	YPtr GlobalBinding::casting(YPtr obj, const uint64_t target)
	{
		if (target == YObjectType::Bool)
		{
			bool data = true;
			if (obj->type_id == YObjectType::Null)
				data = false;
			else if (obj->type_id == YObjectType::Bool)
				data = *(bool*)obj->data;
			else if (obj->type_id == YObjectType::I32 && (*(I32*)obj->data == 0))
				data = false;
			return std::make_shared<YObject>(target, NEW bool(data));
		}
		else if (target == YObjectType::F32)
		{
			Float data = 0.f;
			if (obj->type_id == YObjectType::I32) data = (Float) * (I32*)obj->data;
			else throw std::exception("Unexpected src type");
			return std::make_shared<YObject>(target, NEW Float(data));
		}
		else if (target == YObjectType::String)
		{
			return std::make_shared<YObject>(target, NEW String(__repr__(obj)));
		}
		else throw std::exception("Unexpected target type");
	}

	YPtr GlobalBinding::deepcopy(const YPtr src)
	{
		if (src->type_id == YObjectType::Bool) return std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(bool*)src->data));
		else if (src->type_id == YObjectType::I32) return std::make_shared<YObject>(YObjectType::I32, NEW I32(*(I32*)src->data));
		else if (src->type_id == YObjectType::F32) return std::make_shared<YObject>(YObjectType::F32, NEW Float(*(Float*)src->data));
		else if (src->type_id == YObjectType::String) return std::make_shared<YObject>(YObjectType::String, NEW String(*(String*)src->data));
		else throw std::exception("Unexpected Type");
	}

	YPtr GlobalBinding::operand(const std::string& op, YPtr lhs, YPtr rhs)
	{
		YPtr result = nullptr;
		if (op == "+" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = std::make_shared<YObject>(YObjectType::I32, NEW I32(*(I32*)lhs->data + *(I32*)rhs->data));
		else if (op == "-" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = std::make_shared<YObject>(YObjectType::I32, NEW I32(*(I32*)lhs->data - *(I32*)rhs->data));
		else if (op == "*" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = std::make_shared<YObject>(YObjectType::I32, NEW I32(*(I32*)lhs->data * *(I32*)rhs->data));
		else if (op == "/" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = std::make_shared<YObject>(YObjectType::I32, NEW I32(*(I32*)lhs->data / *(I32*)rhs->data));
		else if (op == "%" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = std::make_shared<YObject>(YObjectType::I32, NEW I32(*(I32*)lhs->data % *(I32*)rhs->data));
		else if (op == "**" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			I32 buffer = 1;
			for (uint64_t k = 0; k < *(I32*)rhs->data; ++k)
				buffer *= *(I32*)lhs->data;
			result = std::make_shared<YObject>(YObjectType::I32, NEW I32(buffer));
		}
		else if (op == ">" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(I32*)lhs->data > * (I32*)rhs->data));
		}
		else if (op == "<" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(I32*)lhs->data < *(I32*)rhs->data));
		}
		else if (op == "==" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(I32*)lhs->data == *(I32*)rhs->data));
		}
		else if (op == "!=" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(I32*)lhs->data != *(I32*)rhs->data));
		}
		else if (op == ">=")
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(!*(bool*)operand("<", lhs, rhs)->data));
		else if (op == "<=")
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(!*(bool*)operand(">", lhs, rhs)->data));
		else if (op == "+" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::F32, NEW Float(*(Float*)lhs->data + *(Float*)rhs->data));
		else if (op == "-" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::F32, NEW Float(*(Float*)lhs->data - *(Float*)rhs->data));
		else if (op == "*" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::F32, NEW Float(*(Float*)lhs->data * *(Float*)rhs->data));
		else if (op == "/" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::F32, NEW Float(*(Float*)lhs->data / *(Float*)rhs->data));
		else if (op == "**" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::F32, NEW Float(powf(*(Float*)lhs->data, *(Float*)rhs->data)));
		else if (op == ">" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(Float*)lhs->data > * (Float*)rhs->data));
		else if (op == "<" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(Float*)lhs->data < *(Float*)rhs->data));
		else if (op == "==" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(Float*)lhs->data == *(Float*)rhs->data));
		else if (op == "!=" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(Float*)lhs->data != *(Float*)rhs->data));
		else if ((op == "+" || op == "-" || op == "*" || op == "/" || op == "**" ||
			op == "==" || op == "!=" || op == ">" || op == "<")
			&& (lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::I32 ||
				lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::F32)
			)
		{
			if (lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::I32)
				result = operand(op, lhs, casting(rhs, YObjectType::F32));
			else
				result = operand(op, casting(lhs, YObjectType::F32), rhs);
		}
		else if (op == "==" && lhs->type_id == YObjectType::String && rhs->type_id == YObjectType::String)
			result = std::make_shared<YObject>(YObjectType::Bool, NEW bool(*(std::string*)lhs->data == *(std::string*)rhs->data));
		else throw std::exception(("Unsupport Operator for : " + std::to_string(lhs->type_id) + " " + op + " " + std::to_string(rhs->type_id)).c_str());
		return result;
	}
}