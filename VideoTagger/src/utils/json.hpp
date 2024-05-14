#pragma once
#include <filesystem>
#include <nlohmann/json.hpp>

namespace vt::utils::json
{
	nlohmann::ordered_json load_from_file(const std::filesystem::path& filepath);
	bool write_to_file(const nlohmann::ordered_json& data, const std::filesystem::path& filepath, bool compact = false);
}
