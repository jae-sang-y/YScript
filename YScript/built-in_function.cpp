#include "yscript.hpp"

namespace YScript
{
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

	YObject* literal_decode(const std::string& src)
	{
		if (src == "true")
		{
			return NEW YObject(YObjectType::Bool, NEW bool(true));
		}
		else if (src == "false")
		{
			return NEW YObject(YObjectType::Bool, NEW bool(false));
		}
		else if (src.at(0) == '"')
		{
			std::string* str = NEW std::string();
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
			return NEW YObject(YObjectType::String, str);
		}
		else
		{
			if (src.find('.') == -1)
			{
				auto obj = NEW YObject(YObjectType::I32, NEW int32_t(std::stof(src)));

				char buffer[256];
				sprintf_s(buffer, "LITEAL 0x%08x \t%s\n", obj->object_id, __repr__(obj));
				OutputDebugStringA(buffer);

				return obj;
			}
			else
			{
				return NEW YObject(YObjectType::F32, NEW float(std::stoi(src)));
			}
		}
	}

	std::string __repr__(YObject* obj)
	{
		switch (obj->type_id)
		{
		case YObjectType::I32: return "I32 " + std::to_string(*(int32_t*)obj->data);
		case YObjectType::F32: return "F32 " + std::to_string(*(float*)obj->data);
		case YObjectType::Bool: return "Bool " + (*(bool*)obj->data) ? "true" : "false";
		case YObjectType::Function: return "Function 0x" + std::to_string(*(uint64_t*)obj->data);
		case YObjectType::String: return "\'" + literal_encode(*(std::string*)obj->data) + "\'";
		}
		if (auto itr = YObjectTypename.find(obj->type_id); itr != YObjectTypename.end())
			return itr->second + " 0x" + std::to_string(obj->object_id);
		return "YObject 0x" + std::to_string(obj->object_id);
	}

	YObject* casting(YObject* obj, const uint64_t target)
	{
		if (target == YObjectType::Bool)
		{
			bool data = true;
			if (obj->type_id == YObjectType::Null)
				data = false;
			else if (obj->type_id == YObjectType::Bool)
				data = *(bool*)obj->data;
			else if (obj->type_id == YObjectType::I32 && (*(int32_t*)obj->data == 0))
				data = false;
			return NEW YObject(target, NEW bool(data));
		}
		else if (target == YObjectType::F32)
		{
			float data = 0.f;
			if (obj->type_id == YObjectType::I32) data = (float)*(int32_t*)obj->data;
			else throw std::exception("Unexpected src type");
			return NEW YObject(target, NEW float(data));
		}
		else if (target == YObjectType::String)
		{
			return NEW YObject(target, NEW std::string(__repr__(obj)));
		}
		else throw std::exception("Unexpected target type");
	}

	YObject* deepcopy(const YObject* src)
	{
		if (src->type_id == YObjectType::Bool) return NEW YObject(YObjectType::Bool, NEW bool(*(bool*)src->data));
		else if (src->type_id == YObjectType::I32) return NEW YObject(YObjectType::I32, NEW int32_t(*(int32_t*)src->data));
		else if (src->type_id == YObjectType::F32) return NEW YObject(YObjectType::F32, NEW float(*(float*)src->data));
		else if (src->type_id == YObjectType::String) return NEW YObject(YObjectType::String, NEW std::string(*(std::string*)src->data));
		else throw std::exception("Unexpected Type");
	}

	YObject* operand(const std::string& op, YObject* lhs, YObject* rhs)
	{
		YObject* result = nullptr;
		if (op == "+" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = NEW YObject(YObjectType::I32, NEW int32_t(*(int32_t*)lhs->data + *(int32_t*)rhs->data));
		else if (op == "-" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = NEW YObject(YObjectType::I32, NEW int32_t(*(int32_t*)lhs->data - *(int32_t*)rhs->data));
		else if (op == "*" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = NEW YObject(YObjectType::I32, NEW int32_t(*(int32_t*)lhs->data * *(int32_t*)rhs->data));
		else if (op == "/" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = NEW YObject(YObjectType::I32, NEW int32_t(*(int32_t*)lhs->data / *(int32_t*)rhs->data));
		else if (op == "%" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
			result = NEW YObject(YObjectType::I32, NEW int32_t(*(int32_t*)lhs->data % *(int32_t*)rhs->data));
		else if (op == "**" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			int32_t buffer = 1;
			for (uint64_t k = 0; k < *(int32_t*)rhs->data; ++k)
				buffer *= *(int32_t*)lhs->data;
			result = NEW YObject(YObjectType::I32, NEW int32_t(buffer));
		}
		else if (op == ">" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			result = NEW YObject(YObjectType::Bool, NEW bool(*(int32_t*)lhs->data > * (int32_t*)rhs->data));
		}
		else if (op == "<" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			result = NEW YObject(YObjectType::Bool, NEW bool(*(int32_t*)lhs->data < *(int32_t*)rhs->data));
		}
		else if (op == "==" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			result = NEW YObject(YObjectType::Bool, NEW bool(*(int32_t*)lhs->data == *(int32_t*)rhs->data));
		}
		else if (op == "!=" && lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::I32)
		{
			result = NEW YObject(YObjectType::Bool, NEW bool(*(int32_t*)lhs->data != *(int32_t*)rhs->data));
		}
		else if (op == ">=")
		{
			YObject* buffer = operand("<", lhs, rhs);
			result = NEW YObject(YObjectType::Bool, NEW bool(!*(bool*)buffer->data));
			delete buffer;
		}
		else if (op == "<=")
		{
			YObject* buffer = operand(">", lhs, rhs);
			result = NEW YObject(YObjectType::Bool, NEW bool(!*(bool*)buffer->data));
			delete buffer;
		}
		else if (op == "+" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::F32, NEW float(*(float*)lhs->data + *(float*)rhs->data));
		else if (op == "-" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::F32, NEW float(*(float*)lhs->data - *(float*)rhs->data));
		else if (op == "*" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::F32, NEW float(*(float*)lhs->data * *(float*)rhs->data));
		else if (op == "/" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::F32, NEW float(*(float*)lhs->data / *(float*)rhs->data));
		else if (op == "**" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::F32, NEW float(powf(*(float*)lhs->data, *(float*)rhs->data)));
		else if (op == ">" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::Bool, NEW bool(*(float*)lhs->data > * (float*)rhs->data));
		else if (op == "<" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::Bool, NEW bool(*(float*)lhs->data < *(float*)rhs->data));
		else if (op == "==" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::Bool, NEW bool(*(float*)lhs->data == *(float*)rhs->data));
		else if (op == "!=" && lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::F32)
			result = NEW YObject(YObjectType::Bool, NEW bool(*(float*)lhs->data != *(float*)rhs->data));
		else if ((op == "+" || op == "-" || op == "*" || op == "/" || op == "**" ||
			op == "==" || op == "!=" || op == ">" || op == "<")
			&& (lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::I32 ||
				lhs->type_id == YObjectType::I32 && rhs->type_id == YObjectType::F32)
			)
		{
			if (lhs->type_id == YObjectType::F32 && rhs->type_id == YObjectType::I32) {
				YObject* casted = casting(rhs, YObjectType::F32);
				result = operand(op, lhs, casted);
				delete casted;
			}
			else {
				YObject* casted = casting(lhs, YObjectType::F32);
				result = operand(op, casted, rhs);
				delete casted;
			}
		}
		else throw std::exception(("Unsupport Operator for : " + std::to_string(lhs->type_id) + " " + op + " " + std::to_string(rhs->type_id)).c_str());
		delete lhs;
		delete rhs;
		return result;
	}

	std::string print(YObject** obj, size_t number)
	{
		std::string result = "";
		for (size_t k = 0; k < number; ++k)
		{
			result += __repr__(obj[k]);
			result.push_back('\r');
			result.push_back('\n');
		}
		return result;
	}
}