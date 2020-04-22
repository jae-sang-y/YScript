#include "yscript.hpp"

using namespace YScript;

Lexer::Lexer(std::string::const_iterator begin, std::string::const_iterator end) {
	for (std::string::const_iterator csr = begin; csr != end; ++csr)
	{
		if (passing_count > 0)
		{
			--passing_count;
			continue;
		}

		if (lexing_state == LexingState::Logic)
		{
			if (*csr == '"')
			{
				lexing_state = LexingState::DoubleQuotesString;
				goto next_loop;
			}
			else if (*csr == '\'')
			{
				lexing_state = LexingState::SingleQuotesString;
				goto next_loop;
			}
			if (match("//", csr, end))
			{
				passing_count += 1;
				lexing_state = LexingState::CommentLine;
				goto next_loop;
			}
			if (match("/*", csr, end))
			{
				passing_count += 1;
				lexing_state = LexingState::CommentLines;
				goto next_loop;
			}

			if (leftover.size() == 0)
			{
				for (const auto& keyword : __keywords__)
				{
					if (match(keyword, csr, end))
					{
						if (leftover.size())
						{
#ifdef DEBUG_LEXING
							std::cout << "VAL: " << leftover << '\n';
#endif
							tokens.push_back(Token{ TokenType::Value, leftover });
							leftover.clear();
						}
						passing_count += keyword.size() - 1;
#ifdef DEBUG_LEXING
						std::cout << "KEY: " << keyword << '\n';
#endif
						tokens.push_back(Token{ TokenType::KeyWord, keyword });

						goto next_loop;
					}
				}
			}
			for (const auto& op : __ops__)
			{
				if (op.size() > 1 && match(op, csr, end) || op.size() == 1 && *csr == op.at(0))
				{
					if (leftover.size())
					{
#ifdef DEBUG_LEXING
						std::cout << "VAL: " << leftover << '\n';
#endif
						tokens.push_back(Token{ TokenType::Value, leftover });
						leftover.clear();
					}
					passing_count += op.size() - 1;
#ifdef DEBUG_LEXING
					std::cout << "OPE: " << op << '\n';
#endif
					tokens.push_back(Token{ TokenType::Operator, op });
					goto next_loop;
				}
			}

			if (is_space(*csr))
			{
				if (leftover.size())
				{
#ifdef DEBUG_LEXING
					std::cout << "VAL: " << leftover << '\n';
#endif
					tokens.push_back(Token{ TokenType::Value, leftover });
					leftover.clear();
				}
			}
			else
			{
				leftover.push_back(*csr);
			}
		}
		else if (lexing_state == LexingState::DoubleQuotesString || lexing_state == LexingState::SingleQuotesString)
		{
			for (const auto pair : __string_literal_pairs__)
			{
				if (match(pair.first, csr, end))
				{
					passing_count += pair.first.size() - 1;
					string_builder.push_back(pair.second);
					goto next_loop;
				}
			}
			if (lexing_state == LexingState::DoubleQuotesString && *csr == '"' ||
				lexing_state == LexingState::SingleQuotesString && *csr == '\'')
			{
				lexing_state = LexingState::Logic;
#ifdef DEBUG_LEXING
				std::cout << "VAL: " << "\"" << string_builder << "\"" << '\n';
#endif
				tokens.push_back(Token{ TokenType::Value, string_builder });
				string_builder.clear();
				goto next_loop;
			}
			string_builder.push_back(*csr);
		}
		else if (lexing_state == LexingState::CommentLine)
		{
			if (*csr == '\n')
				lexing_state = LexingState::Logic;
		}
		else if (lexing_state == LexingState::CommentLines)
		{
			if (match("*/", csr, end))
			{
				passing_count += 1;
				lexing_state = LexingState::Logic;
			}
		}

	next_loop:;
	}
}

bool Lexer::match(const std::string& token, std::string::const_iterator itr, std::string::const_iterator end) {
	for (std::string::const_iterator cursor = token.cbegin(); ;)
	{
		if (cursor == token.end())
			return true;

		if (itr == end)
			return false;
		if ((*itr) != (*cursor))
			return false;

		++cursor;
		++itr;
	}
}

bool Lexer::is_space(char ch)
{
	return (ch == '\r' || ch == '\n' || ch == '\t' || ch == '\f' || ch == '\v' || ch == ' ');
}