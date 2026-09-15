#pragma once
#include <string>
#include <string_view>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace vt::utils::json
{
	nlohmann::ordered_json from_string(std::string_view input);
	nlohmann::ordered_json from_string(const std::string& input);
	nlohmann::ordered_json load_from_file(const std::filesystem::path& filepath);
	bool write_to_file(const nlohmann::ordered_json& data, const std::filesystem::path& filepath, bool compact = false);
}
