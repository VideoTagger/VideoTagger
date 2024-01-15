#include "string.hpp"
#include <numeric>
#include <vector>
#include <algorithm>

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
					distances[j + 1] = std::min({ std::exchange(previous_distance, distances[j + 1]) + (left[i] == right[j] ? 0 : 1), distances[j] + 1, distances[j + 1] + 1 });
				}
			}
			return distances[size_b];
	}

	std::string to_lowercase(const std::string& input)
	{
		std::string result = input;
		std::transform(input.begin(), input.end(), result.begin(), tolower);
		return result;
	}

	std::string to_uppercase(const std::string& input)
	{
		std::string result = input;
		std::transform(input.begin(), input.end(), result.begin(), toupper);
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
