#include <filesystem>
#include <string>

namespace vt::utils::string
{
	uint64_t levenshtein_dist(const std::string& left, const std::string& right);
	
	std::string to_lowercase(const std::string& input);
	std::string to_uppercase(const std::string& input);
	std::string trim_whitespace(const std::string& input);
	std::vector<std::string> split(const std::string& input, char delimiter);

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
