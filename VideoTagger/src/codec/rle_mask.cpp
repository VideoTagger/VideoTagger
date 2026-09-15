#include "rle_mask.hpp"
#include <core/debug.hpp>
#include <utils/vec.hpp>
#include <codec/base64.hpp>
#include <codec/uleb128.hpp>

namespace vt::codec
{
	rle_mask rle_mask::encode(const uint8_t* mask_data, const utils::vec2<int>& size)
	{
		rle_mask mask;
		mask.size = size;

		std::vector<uint32_t>& runs = mask.runs;
		if (mask_data == nullptr) return mask;

		size_t len = size[0] * size[1];
		if (len == 0) return mask;

		mask.start_value = mask_data[0] != 0;
		bool current = mask.start_value;
		uint32_t count = 0;

		// column-major order
		for (int x = 0; x < size[0]; ++x)
		{
			for (int y = 0; y < size[1]; ++y)
			{
				bool pixel = mask_data[y * size[0] + x] != 0;
				if (pixel == current)
				{
					count++;
				}
				else
				{
					runs.push_back(count);
					current = pixel;
					count = 1;
				}
			}
		}

		if (count > 0)
		{
			runs.push_back(count);
		}
		return mask;
	}

	std::vector<uint8_t> rle_mask::decode(const rle_mask& mask_data)
	{
		std::vector<uint8_t> result;
		int width = mask_data.size[0];
		int height = mask_data.size[1];

		result.resize(width * height);
		uint8_t value = static_cast<uint8_t>(mask_data.start_value);
		size_t i = 0;

		for (uint32_t run : mask_data.runs)
		{
			for (uint32_t j = 0; j < run; ++j)
			{
				if (i >= result.size())
				{
					return result;
				}
				
				int x = i / height;
				int y = i % height;

				//row major indexing
				result[(y * width) + x] = value * 0xFF;

				i++;
			}
			value ^= 1;
		}
		return result;
	}

	nlohmann::ordered_json rle_mask::serialize() const
	{
		nlohmann::ordered_json json;
		json["size"] = size;
		json["start-value"] = start_value;

		std::vector<uint8_t> encoded_runs;
		for (uint32_t run : runs)
		{
			uleb128::encode(run, encoded_runs);
		}
		json["data"] = base64::encode(encoded_runs, base64::encode_table::normal, true);
		return json;
	}

	void rle_mask::deserialize(const nlohmann::ordered_json& json)
	{
		if (!json.contains("size") or !json["size"].is_array() or json["size"].size() != 2)
		{
			debug::error("Invalid JSON: missing or invalid 'size' field");
			return;
		}
		size = json["size"].get<utils::vec2<int>>();

		if (!json.contains("start-value") or !json["start-value"].is_boolean())
		{
			debug::error("Invalid JSON: missing or invalid 'start-value' field");
			return;
		}
		start_value = json["start-value"].get<bool>();
		if (!json.contains("data") or !json["data"].is_string())
		{
			debug::error("Invalid JSON: missing or invalid 'data' field");
			return;
		}

		std::string encoded_data = json["data"].get<std::string>();
		std::vector<uint8_t> decoded_data = base64::decode(encoded_data);
		size_t offset{};
		while (offset < decoded_data.size())
		{
			runs.push_back(uleb128::decode(decoded_data, offset));
		}
	}
}
