#pragma once
#include <filesystem>
#include <cstdint>
#include <string_view>
#include <vector>

namespace vt::utils::hash
{
	extern uint64_t fnv_hash(const std::filesystem::path& filepath);

	extern std::vector<uint8_t> sha256(std::string_view string);
}
