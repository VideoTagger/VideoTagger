#include "uleb128.hpp"

namespace vt::codec::uleb128
{
	void encode(uint32_t value, std::vector<uint8_t>& output)
	{
		while (true)
		{
			uint8_t byte = value & 0x7F;
			value >>= 7;

			if (value != 0)
			{
				byte |= 0x80;
			}
			output.push_back(byte);
			if (value == 0) break;
		}
	}

	uint32_t decode(const std::vector<uint8_t>& input, size_t& offset)
	{
		uint32_t result = 0;
		uint32_t shift = 0;

		while (true)
		{
			uint8_t byte = input[offset++];
			result |= (byte & 0x7F) << shift;

			if ((byte & 0x80) == 0) break;

			shift += 7;
		}

		return result;
	}
}
