#pragma once
#include <filesystem>
#include <cstdint>
#include <string_view>
#include <vector>

namespace vt::utils::hash
{
	extern uint64_t fnv_hash(const std::filesystem::path& filepath);

	static constexpr auto sha256_byte_count = 32;

	extern std::vector<uint8_t> sha256(std::string_view string);
	extern std::vector<uint8_t> sha256_file(const std::filesystem::path& filepath);

	enum class string_case
	{
		upper,
		lower
	};

	template<typename Container>
	inline std::string bytes_to_hex(const Container& bytes, string_case result_case);
	extern std::vector<uint8_t> hex_to_bytes(std::string_view hex_string);

	template<typename Container>
	inline std::string bytes_to_hex(const Container& bytes, string_case result_case)
	{
		std::ostringstream ss;
		ss << std::hex << (result_case == string_case::upper ? std::uppercase : std::nouppercase);
		for (auto& byte : bytes)
		{
			ss << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
		}

		return ss.str();
	}
}
