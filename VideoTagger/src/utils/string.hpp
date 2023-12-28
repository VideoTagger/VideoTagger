#include <filesystem>
#include <string_view>

namespace vt::utils::string
{
	uint64_t levenshtein_dist(const std::string& left, const std::string& right, int r_length, int l_length);
	
	std::string to_lowercase(const std::string& input);
	std::string to_uppercase(const std::string& input);
	std::string trim_whitespace(const std::string& input);
	std::vector<std::string> split(const std::string& input, char delimiter);
}
