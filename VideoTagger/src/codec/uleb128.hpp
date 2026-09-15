#pragma once
#include <vector>
#include <cstdint>

namespace vt::codec::uleb128
{
	void encode(uint32_t value, std::vector<uint8_t>& output);
	uint32_t decode(const std::vector<uint8_t>& input, size_t& offset);
}
