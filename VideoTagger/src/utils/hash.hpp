#pragma once
#include <filesystem>
#include <cstdint>

namespace vt::utils::hash
{
	uint64_t fnv_hash(const std::filesystem::path& filepath);
}
