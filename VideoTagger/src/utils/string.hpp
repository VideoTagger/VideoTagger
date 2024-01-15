#include <filesystem>
#include <string>

namespace vt::utils::string
{
	uint64_t levenshtein_dist(const std::string& left, const std::string& right);
	
	std::string to_lowercase(const std::string& input);
	std::string to_uppercase(const std::string& input);
	std::string trim_whitespace(const std::string& input);
	std::vector<std::string> split(const std::string& input, char delimiter);
}
