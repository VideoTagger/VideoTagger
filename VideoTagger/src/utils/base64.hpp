#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace vt::utils::base64
{
	enum class base64_table
	{
		normal,
		url
	};

	extern std::string encode(const std::vector<uint8_t>& data, base64_table table, bool remove_padding = false);
	extern std::vector<uint8_t> decode(const std::string& encoded_string);
}
