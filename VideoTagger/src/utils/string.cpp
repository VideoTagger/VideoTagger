#include "pch.hpp"
#include "string.hpp"

namespace vt::utils::string
{
	uint64_t levenshtein_dist(const std::string& left, const std::string& right)
	{
		const auto size_a = left.size();
		const auto size_b = right.size();

		std::vector<std::size_t> distances(size_b + 1);
		std::iota(distances.begin(), distances.end(), std::size_t{});

		for (std::size_t i = 0; i < size_a; ++i) 
		{
			std::size_t previous_distance = 0;
			for (std::size_t j = 0; j < size_b; ++j) 
			{
				distances[j + 1] = std::min(std::exchange(previous_distance, distances[j + 1]) + (left[i] == right[j] ? 0 : 1), std::min(distances[j] + 1, distances[j + 1] + 1));
			}
		}
		return distances[size_b];
	}

	std::string replace_all(const std::string& input, const std::string& from, const std::string& to)
	{
		std::string result;
		result.reserve(input.size());

		size_t last_pos{};
		size_t find_pos{};

		while ((find_pos = input.find(from, last_pos)) != std::string::npos)
		{
			result.append(input, last_pos, find_pos - last_pos);
			result += to;
			last_pos = find_pos + from.length();
		}

		result += input.substr(last_pos);
		return result;
	}

	std::string to_lowercase(const std::string& input)
	{
		std::string result = input;
		if (!result.empty())
		{
			utf8lwr(result.data());
		}
		return result;
	}

	std::string to_uppercase(const std::string& input)
	{
		std::string result = input;
		if (!result.empty())
		{
			utf8upr(result.data());
		}
		return result;
	}

    std::string to_titlecase(const std::string& input)
    {
		std::string result;
		bool capitalize_next = true;

		for (size_t i = 0; i < input.size();)
		{
			utf8_int32_t c;
			auto char_ptr = input.data() + i;
			utf8codepoint(char_ptr, &c);
			auto c_size = utf8codepointsize(c);

			if (capitalize_next and std::isalpha(c))
			{
				std::string upper_char = to_uppercase(input.substr(i, c_size));
				result += upper_char;
				capitalize_next = false;
			}
			else
			{
				result += input.substr(i, c_size);
			}

			if (std::isspace(c) or std::ispunct(c))
			{
				capitalize_next = true;
			}

			i += c_size;
		}
		return result;
    }

	static void ltrim(std::string& inout)
	{
		inout.erase(inout.begin(), std::find_if(inout.begin(), inout.end(), [](unsigned char c) { return !std::isspace(c); }));
	}

	static void rtrim(std::string& inout)
	{
		inout.erase(std::find_if(inout.rbegin(), inout.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), inout.end());
	}

	std::string trim_whitespace(const std::string& input)
	{
		std::string result = input;
		ltrim(result);
		rtrim(result);
		return result;
	}

	std::vector<std::string> split(const std::string& input, char delimiter)
	{
		std::vector<std::string> result;
		if (input.find(delimiter) >= input.size()) return { input };

		size_t prev_pos{};
		size_t pos{};

		while ((pos = input.find(delimiter, pos)) != std::string::npos)
		{
			result.push_back(input.substr(prev_pos, pos - prev_pos));
			prev_pos = ++pos;
		}
		result.push_back(input.substr(prev_pos, pos - prev_pos));
		return result;
	}
}
