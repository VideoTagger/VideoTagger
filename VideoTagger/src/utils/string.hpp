#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>


namespace vt::utils::string
{
	uint64_t levenshtein_dist(const std::string& left, const std::string& right);
	std::string replace_all(const std::string& input, const std::string& from, const std::string& to);
	
	std::string to_lowercase(const std::string& input);
	std::string to_uppercase(const std::string& input);
	std::string trim_whitespace(const std::string& input);
	std::vector<std::string> split(const std::string& input, char delimiter);
	
	// The signature of the function must be equivalent to: void f(std::string_view);
	// If the input string doesn't contain *delimiter*, *func* will be called once with the whole string as the argument
	// The std::string_view passed to *func* doesn't contain *delimiter*
	template<typename Function>
	void for_each_line(std::string_view input, Function func, char delimiter = '\n');

	template <typename type> std::string to_hex(type input, size_t hex_length = sizeof(type) << 1)
	{
		static constexpr const char* digits = "0123456789abcdef";
		std::string result(hex_length, '0');
		for (size_t i = 0, j = (hex_length - 1) * 4; i < hex_length; ++i, j -= 4)
		{
			result[i] = digits[(input >> j) & 0x0F];
		}
		return result;
	}

	template<typename Function>
	void for_each_line(std::string_view input, Function func, char delimiter)
	{
		while (!input.empty())
		{
			size_t separator_index = input.find(delimiter);
			if (separator_index == input.npos)
			{
				separator_index = input.size();
			}

			func(input.substr(0, separator_index));

			input.remove_prefix(std::min(separator_index + 1, input.size()));
		}
	}
}
