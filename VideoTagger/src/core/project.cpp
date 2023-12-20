#include "project.hpp"
#include <vector>
#include <utils/json.hpp>

namespace vt
{
    bool project::is_valid() const
	{
		return !path.empty() and !name.empty();
	}

	std::tm project::modification_time() const
	{
		return std::tm();
	}
	
	void project::save() const
	{
		//TODO: Add error checking

		nlohmann::ordered_json json;
		auto& project = json["project"];
		project["name"] = name;
		project["working-dir"] = std::filesystem::relative(working_dir);

		//TODO: Tags
		auto& tags = json["tags"];
		tags = nlohmann::json::array();

		//TODO: Keybinds
		auto& keybinds = json["keybinds"];
		keybinds = nlohmann::json::array();
		auto parent = path.parent_path();
		std::filesystem::create_directories(parent);
		utils::json::write_to_file(json, path);
	}

	bool project::operator==(const project& other) const
	{
		return name == other.name and path == other.path and working_dir == other.working_dir;
	}
	
	project project::load_from_file(const std::filesystem::path& filepath)
	{
		//TODO: Add error checking

		project result;
		nlohmann::json json = utils::json::load_from_file(filepath);
		const auto& project = json["project"];
		result.path = filepath;
		result.name = project["name"];
		result.working_dir = project["working-dir"].get<std::filesystem::path>();
		return result;
	}
}
