#include <vector>
#include <ostream>
#include <string>
#include <stack>
#include <unordered_map>
#include <iostream>

#define DEBUG_TREE

#ifdef _DEBUG
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

#ifndef __TREE__

#ifdef DEBUG_TREE
namespace debug_tree {
	struct PrintData {
		uint64_t required_space = -1;
		uint64_t its_position = -1;
	};

	struct DebugFloorData {
		std::unordered_map<uint64_t, std::string> names = std::unordered_map<uint64_t, std::string>();
		std::unordered_map<uint64_t, uint8_t> brackets = std::unordered_map<uint64_t, uint8_t>();
		uint64_t max_width = 4;
		constexpr static uint8_t OPEN = 1;
		constexpr static uint8_t CLOSE = 2;
	};

	struct TreeAnalyzeData {
		std::vector<DebugFloorData> floors;
		uint64_t max_height = 1;

		void Draw(std::ostream& out)
		{
			out << "\n=";
			std::vector<bool> rains;
			rains.resize(max_height + 1);

			for (uint64_t y = 0; y <= max_height; ++y)
			{
				int x = -1;
				for (const DebugFloorData& floor : floors)
				{
					const auto itr = floor.brackets.find(y);
					bool lazy_set_false = false;

					x += 1;

					if (itr != floor.brackets.cend())
					{
						if (itr->second & DebugFloorData::OPEN)
							rains.at(x) = true;
						if (itr->second & DebugFloorData::CLOSE)
							lazy_set_false = true;
					}

					if (const auto itr = floor.names.find(y); itr != floor.names.end())
					{
						out << itr->second;
						for (uint64_t k = itr->second.size(); k <= floor.max_width; ++k) out << " ";
						if (rains.at(x))
							out << "| ";
						else
							out << "  ";
					}
					else
					{
						for (uint64_t k = 0; k < floor.max_width; ++k) out << " ";
						if (rains.at(x))
							out << " | ";
						else
							out << "   ";
					}

					if (lazy_set_false)
						rains.at(x) = false;
				}
				out << "\n=";
			}
			out << "\n\n";
		}
	};
};
#endif

template <typename T>
struct tree {
	std::list<tree<T>*> childs;
	T body;

	tree<T>(T body) : body(body) {
	}

	tree<T>* push_back(const T& body)
	{
		childs.push_back(NEW tree<T>(body));
		return *(--childs.end());
	}

	~tree<T>() {
		for (tree<T>* child : childs)
		{
			delete child;
		}
	}

#ifdef DEBUG_TREE
	debug_tree::PrintData analyze(debug_tree::TreeAnalyzeData* data, uint64_t x, uint64_t& y)
	{
		if (childs.size() > 0)
		{
			uint64_t y_for_child = y;
			uint64_t required_space = -1;
			uint64_t position = -1;
			uint64_t child_begin_position = -1;
			uint64_t child_end_position = -1;
			for (tree<T>* child : childs)
			{
				if (required_space != -1)
					y_for_child += 2;
				if (x + 1 >= data->floors.size())
					data->floors.push_back(debug_tree::DebugFloorData());

				debug_tree::PrintData ret = child->analyze(data, x + 1, y_for_child);

				if (required_space == -1)
					child_begin_position = ret.its_position;
				child_end_position = ret.its_position;

				required_space += ret.required_space + 1;
			}

			auto& floor = data->floors.at(x);

			position = (y + y_for_child) / 2;

			std::string text = (body.str.size() ? body.str : "NULL");
			if (text.size() > floor.max_width)
				floor.max_width = text.size();
			floor.names[position] = text;

			if (child_begin_position != child_end_position)
			{
				floor.brackets[child_begin_position] = debug_tree::DebugFloorData::OPEN;
				floor.brackets[child_end_position] = debug_tree::DebugFloorData::CLOSE;
			}
			else
			{
				floor.brackets[child_end_position] = debug_tree::DebugFloorData::OPEN | debug_tree::DebugFloorData::CLOSE;
			}

			y = y_for_child;

			return debug_tree::PrintData{ required_space, position };
		}
		else
		{
			if (x >= data->floors.size())
				data->floors.push_back(debug_tree::DebugFloorData());
			if (y > data->max_height)
				data->max_height = y;

			auto& floor = data->floors.at(x);

			if (body.str.size())
			{
				floor.names[y] = body.str;
				if (body.str.size() + 1 > floor.max_width)
					floor.max_width = body.str.size();
			}
			else
			{
				floor.names[y] = "NULL;";
			}

			return debug_tree::PrintData{ 1, y };
		}
	}
	int get_depth(int y = 0)
	{
		if (childs.size() > 0)
		{
			int max = 0;
			for (tree<T>* child : childs)
			{
				int k = child->get_depth(y + 1);
				if (k > max) max = k;
			}
			return max;
		}
		else return y + 1;
	}
#endif
};

#define __TREE__
#endif