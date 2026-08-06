#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace vt::utils::string
{
	/**
	 * @brief Calculates the Levenshtein distance between two strings
	 * @param[in] left First string
	 * @param[in] right Second string
	 * 
	 * @return The Levenshtein distance between `left` and `right`
	 */
	size_t levenshtein_dist(const std::string& left, const std::string& right);
	
	/**
	 * @brief Replaces all occurrences of a substring in a string with another substring
	 * @param[in] input Source string
	 * @param[in] from Substring to be replaced
	 * @param[in] to String replacement
	 * 
	 * @return New string with all occurrences of `from` replaced with `to`
	 */
	std::string replace_all(const std::string& input, const std::string& from, const std::string& to);
	
	/**
	 * @brief Turns all characters in a string to lowercase
	 * @param[in] input Source string
	 * 
	 * @return New string with all lowercase characters
	 */
	std::string to_lowercase(const std::string& input);
	
	/**
	 * @brief Turns all characters in a string to uppercase
	 * @param[in] input Source string
	 * 
	 * @return New string with all uppercase characters
	 */
	std::string to_uppercase(const std::string& input);

	/**
	 * @brief Turns the first character of each word in a string to uppercase and the rest to lowercase
	 * @param[in] input Source string
	 * 
	 * @return New string with titlecase formatting
	 */
	std::string to_titlecase(const std::string& input);

	/**
	 * @brief Trims leading and trailing whitespace from a string
	 * @param[in] input Source string
	 * 
	 * @return New string with leading and trailing whitespace removed
	 */
	std::string trim_whitespace(const std::string& input);

	/**
	 * @brief Checks if a string has leading or trailing whitespace
	 * @param[in] input Source string
	 * 
	 * @return true if the string has leading or trailing whitespace, false otherwise
	 */
	bool has_trailing_whitespace(const std::string& input);

	/**
	 * @brief Splits the `input` string into a vector of strings based on a character delimiter
	 * @param[in] input Source string
	 * @param[in] delimiter Character used to split the string
	 * 
	 * @return Vector of strings split by the delimiter
	 */
	std::vector<std::string> split(const std::string& input, char delimiter);

	/**
	 * @brief Converts a numeric type to a hexadecimal string representation
	 * @param[in] input Numeric value to convert
	 * @param[in] hex_length Length of the resulting hexadecimal string
	 * @tparam type Numeric type to convert
	 * 
	 * @return Hexadecimal string representation of the `input` value
	 */
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
}
