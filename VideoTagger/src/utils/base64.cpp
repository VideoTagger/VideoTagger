#include "pch.hpp"
#include "base64.hpp"

namespace vt::utils::base64
{
	std::string encode(const std::vector<uint8_t>& data, base64_table table, bool remove_padding)
	{
		static constexpr const char* encode_tables[] =
		{
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
		};
		static constexpr char padding_chars[] = { '=', '.' };

		size_t input_size = data.size();
		size_t output_size = 4 * ((input_size + 2) / 3);
		std::string result;
		result.reserve(output_size);

		const auto& encode_table = encode_tables[(size_t)table];
		const auto& padding_char = padding_chars[(size_t)table];

		size_t pos = 0;

		while (pos < input_size)
		{
			result.push_back(encode_table[(data[pos] & 0xfc) >> 2]);

			if (pos + 1 < input_size)
			{
				result.push_back(encode_table[((data[pos] & 0x03) << 4) + ((data[pos + 1] & 0xf0) >> 4)]);

				if (pos + 2 < input_size)
				{
					result.push_back(encode_table[((data[pos + 1] & 0x0f) << 2) + ((data[pos + 2] & 0xc0) >> 6)]);
					result.push_back(encode_table[data[pos + 2] & 0x3f]);
				}
				else
				{
					result.push_back(encode_table[(data[pos + 1] & 0x0f) << 2]);
					result.push_back(padding_char);
				}
			}
			else
			{
				result.push_back(encode_table[(data[pos] & 0x03) << 4]);
				result.push_back(padding_char);
				result.push_back(padding_char);
			}

			pos += 3;
		}

		if (remove_padding)
		{
			result = result.substr(0, result.find(padding_char));
		}
		return result;
	}

	static uint8_t char_pos(const uint8_t c)
	{
		if (c >= 'A' && c <= 'Z') return c - 'A';
		else if (c >= 'a' && c <= 'z') return c - 'a' + ('Z' - 'A') + 1;
		else if (c >= '0' && c <= '9') return c - '0' + ('Z' - 'A') + ('z' - 'a') + 2;
		else if (c == '+' || c == '-') return 62;
		else if (c == '/' || c == '_') return 63;
		else throw std::runtime_error("Input is not valid base64-encoded data.");
	}

	std::vector<uint8_t> decode(const std::string& encoded_string)
	{
		size_t input_size = encoded_string.size();

		size_t out_size = input_size / 4 * 3;
		std::vector<uint8_t> result;
		result.reserve(out_size);

		for (size_t pos = 0; pos < input_size; pos += 4)
		{
			uint8_t cpos1 = char_pos(encoded_string[pos + 1]);

			result.push_back((char_pos(encoded_string[pos + 0]) << 2) + ((cpos1 & 0x30) >> 4));

			if ((pos + 2 < input_size) && encoded_string[pos + 2] != '=' && encoded_string[pos + 2] != '.')
			{
				uint8_t cpos2 = char_pos(encoded_string[pos + 2]);
				result.push_back(((cpos1 & 0x0f) << 4) + ((cpos2 & 0x3c) >> 2));

				if ((pos + 3 < input_size) && encoded_string[pos + 3] != '=' && encoded_string[pos + 3] != '.')
				{
					result.push_back(((cpos2 & 0x03) << 6) + char_pos(encoded_string[pos + 3]));
				}
			}
		}
		return result;
	}
}
