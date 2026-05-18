#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace vt::codec::base64
{
	enum class encode_table
	{
		normal,
		url
	};

	extern std::string encode(const std::vector<uint8_t>& data, encode_table table, bool remove_padding = false);
	extern std::vector<uint8_t> decode(std::string_view encoded_string);
}
