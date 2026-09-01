#pragma once
#include <string>
#include <filesystem>

namespace vt
{
	struct benchmark_download_data
	{
		std::string name;
		std::string url;
		std::filesystem::path output_path;
	};
}
