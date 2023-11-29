#include "json.hpp"
namespace vt::utils::json
{
	nlohmann::json load_from_file(const std::filesystem::path& filepath)
	{
		nlohmann::json b;
		return b;
	}
	void write_to_file(const nlohmann::json& data, const std::filesystem::path& filepath)
	{
	}
}

