#include <vector>
#include <ostream>
#include <string>
#include <stack>

#ifdef DEBUG_TREE
namespace debug_tree {
	struct PrintData {
		size_t required_space = -1;
		size_t its_position = -1;
	};

	struct DebugFloorData {
		std::unordered_map<size_t, std::string> names = std::unordered_map<size_t, std::string>();
		std::unordered_map<size_t, uint8_t> brackets = std::unordered_map<size_t, uint8_t>();
		size_t max_width = 4;
		constexpr static uint8_t OPEN = 1;
		constexpr static uint8_t CLOSE = 2;
	};

	struct TreeAnalyzeData {
		std::vector<DebugFloorData> floors;
		size_t max_height = 0;

		void Draw(std::ostream& out)
		{
			out << "\n=";
			std::vector<bool> rains;
			rains.resize(max_height + 1);

			for (size_t y = 0; y <= max_height; ++y)
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
						for (size_t k = itr->second.size(); k <= floor.max_width; ++k) out << " ";
						if (rains.at(x))
							out << "| ";
						else
							out << "  ";
					}
					else
					{
						for (size_t k = 0; k < floor.max_width; ++k) out << " ";
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
	std::vector<tree<T>> childs;
	T body;
	tree<T>* root;

	tree<T>(T body, tree<T>* root = nullptr) : root(root), body(body) {
	}

	tree<T>& push_back(const T& body)
	{
		tree<T> t(body, this);
		childs.push_back(t);

		return *(childs.end() - 1);
	}

#ifdef DEBUG_TREE
	debug_tree::PrintData analyze(debug_tree::TreeAnalyzeData* data, size_t x, size_t& y)
	{
		if (childs.size() > 0)
		{
			size_t y_for_child = y;
			size_t required_space = -1;
			size_t position = -1;
			size_t child_begin_position = -1;
			size_t child_end_position = -1;
			for (tree<T> child : childs)
			{
				if (required_space != -1)
					y_for_child += 2;
				if (x + 1 >= data->floors.size())
					data->floors.push_back(debug_tree::DebugFloorData());

				debug_tree::PrintData ret = child.analyze(data, x + 1, y_for_child);

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
#endif
};