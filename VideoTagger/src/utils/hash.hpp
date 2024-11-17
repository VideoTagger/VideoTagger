#pragma once
#include <filesystem>
#include <cstdint>
#include <string_view>
#include <vector>

namespace vt::utils::hash
{
	extern uint64_t fnv_hash(const std::filesystem::path& filepath);

	extern std::vector<uint8_t> sha256(std::string_view string);
	extern std::vector<uint8_t> sha256_file(const std::filesystem::path& filepath);

	enum class string_case
	{
		upper,
		lower
	};

	extern std::string bytes_to_hex(const std::vector<uint8_t>& bytes, string_case result_case);
	extern std::vector<uint8_t> hex_to_bytes(std::string_view hex_string);
}
