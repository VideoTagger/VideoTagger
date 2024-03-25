#include "hash.hpp"
#include "iostream"
#include "fstream"
#include "vector"
#include "string"
namespace vt::utils::hash
{
	uint64_t fnv_hash(const std::filesystem::path& filepath)
	{
		std::ifstream in(filepath, std::ios::binary);
		uint64_t hash = 14695981039346656037ull;

		while (!in.eof())
		{
			uint8_t byte{};
			in.read((char*)&byte, sizeof(byte));

			hash *= 1099511628211ull;
			hash ^= byte;
		}
		return hash;
	}
}
