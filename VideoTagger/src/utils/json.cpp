#include "json.hpp"
#include <fstream>

#include <core/debug.hpp>

namespace vt::utils::json
{
	nlohmann::ordered_json load_from_file(const std::filesystem::path& filepath)
	{
		nlohmann::ordered_json result;
		std::ifstream file(filepath);
		if (file.is_open())
		{
			file >> result;
		}
		else
		{
			debug::error("Couldn't load Json file: " + filepath.string());
		}
		return result;
	}

	void write_to_file(const nlohmann::ordered_json& data, const std::filesystem::path& filepath, bool compact)
	{
		std::ofstream file(filepath);
		if (file.is_open())
		{
			file << data.dump(compact ? -1 : 1, '\t') << '\n';
		}
		else
		{
			debug::error("Couldn't write to Json file: " + filepath.string());
		}
	}
}

