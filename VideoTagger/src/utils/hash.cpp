#include "hash.hpp"
#include "iostream"
#include "fstream"
#include "vector"
#include "string"
namespace vt::utils::hash
{
	std::vector<char> file_to_byte(const std::filesystem::path& filepath)
	{
		std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
		std::ifstream::pos_type pos = ifs.tellg();

		if (pos == 0) {
			return std::vector<char>{};
		}

		std::vector<char>  result(pos);

		ifs.seekg(0, std::ios::beg);
		ifs.read(&result[0], pos);

		return result;

	}
	uint64_t fnv_hash(const std::filesystem::path& filepath)
	{
		
		 std::vector<char> bytefile = file_to_byte(filepath);
		

		 uint64_t hash = 14695981039346656037; //FNV_offset_basis
		 for (auto fileB : bytefile)
		 {
			 hash *= 1099511628211;//using FNV Prime
			 hash ^= fileB;

		 }
		 std::cout << "hash: "<<hash;

		 //15096513557945575548
		return 0; 
	}

	
	
}
