#pragma once
#include <filesystem>
#include <json.hpp>

namespace vt::utils::json
{
	nlohmann::json load_from_file(const std::filesystem::path& filepath);
	void write_to_file(const nlohmann::json& data, const std::filesystem::path& filepath);
}
