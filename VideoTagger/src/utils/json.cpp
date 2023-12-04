#include "json.hpp"
#include <fstream>
namespace vt::utils::json
{
	nlohmann::json load_from_file(const std::filesystem::path& filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open()) {
			throw std::runtime_error("Filed opening file due to reading");
		}
		nlohmann::json jsonData;
		file >> jsonData;

		return jsonData;;
	}
	void write_to_file(const nlohmann::json& data, const std::filesystem::path& filepath)
	{
		std::ofstream file(filepath);
		if (!file.is_open()) {
			throw std::runtime_error("Filed opening file due to reading");
		}
		file << std::setw(4) << data;
	}
}
