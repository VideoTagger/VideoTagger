#include "string.hpp"
#include <algorithm>

namespace vt::utils::string
{
	uint64_t levenshtein_dist(const std::string& left, const std::string& right, int l_length, int r_length)
	{


		if (l_length == 0) {
			return r_length;
		}
		if (r_length == 0) {
			return l_length;
		}
	
		if (left[l_length - 1] == right[r_length - 1]) {
			return levenshtein_dist(left, right, l_length - 1,
				r_length - 1);
		}
		return 1
			+ std::min(

				
				levenshtein_dist(left, right, l_length, r_length - 1),
				std::min(

				
					levenshtein_dist(left, right, l_length - 1,
						r_length),

					
					levenshtein_dist(left, right, l_length - 1,
						r_length - 1)));

		
		return 0;
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
