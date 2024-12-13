#include "pch.hpp"
#include "hash.hpp"
#include <openssl/sha.h>
#include <openssl/evp.h>

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
				hash ^= file_buffer[i];
				hash *= 1099511628211ull;
			}
		}
		return hash;
	}

	std::vector<uint8_t> sha256(std::string_view string)
	{
		std::vector<uint8_t> result(SHA256_DIGEST_LENGTH, 0);
		SHA256(reinterpret_cast<const unsigned char*>(string.data()), string.size(), result.data());
		return result;
	}

	std::vector<uint8_t> sha256_file(const std::filesystem::path& filepath)
	{
		static constexpr size_t file_buffer_size = 4096;

		std::ifstream in(filepath);
		if (!in.is_open())
		{
			return {};
		}

		EVP_MD_CTX* context = EVP_MD_CTX_new();
		if (context == nullptr)
		{
			return {};
		}

		if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
			EVP_MD_CTX_free(context);
			return {};
		}

		std::array<uint8_t, file_buffer_size> file_buffer{};
		while (!in.eof())
		{
			in.read((char*)file_buffer.data(), file_buffer_size);
			auto read_bytes = in.gcount();
			if (read_bytes == 0)
			{
				break;
			}

			if (EVP_DigestUpdate(context, file_buffer.data(), read_bytes) != 1)
			{
				EVP_MD_CTX_free(context);
				return {};
			}
		}

		std::vector<uint8_t> result(SHA256_DIGEST_LENGTH, 0);
		if (EVP_DigestFinal_ex(context, result.data(), nullptr) != 1)
		{
			EVP_MD_CTX_free(context);
			return {};
		}

		EVP_MD_CTX_free(context);

		return result;
	}

	std::vector<uint8_t> hex_to_bytes(std::string_view hex_string)
	{
		std::vector<uint8_t> result(hex_string.size() / 2);

		for (size_t i = 0; i < result.size(); i++)
		{
			std::stringstream ss(std::string(hex_string.substr(i * 2, 2)));
			ss >> std::hex;
			int byte;
			ss >> byte;
			result[i] = static_cast<uint8_t>(byte);
		}

		return result;
	}
}
