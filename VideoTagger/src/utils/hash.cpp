#include "hash.hpp"
#include <fstream>
#include <string>
#include <array>

namespace vt::utils::hash
{
	uint64_t fnv_hash(const std::filesystem::path& filepath)
	{
		static constexpr size_t file_buffer_size = 4096;

		std::ifstream in(filepath, std::ios::binary);
		if (!in.is_open())
		{
			return 0;
		}

		uint64_t hash = 14695981039346656037ull;

		std::array<uint8_t, file_buffer_size> file_buffer{};
		
		while (!in.eof())
		{
			in.read((char*)file_buffer.data(), file_buffer_size);
			auto read_bytes = in.gcount();
			if (read_bytes == 0)
			{
				break;
			}

			for (std::streamsize i = 0; i < read_bytes; i++)
			{
				hash *= 1099511628211ull;
				hash ^= file_buffer[i];
			}
		}
		return hash;
	}
}
