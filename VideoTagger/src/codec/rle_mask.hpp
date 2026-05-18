#pragma once
#include <cstdint>
#include <vector>

#include <utils/vec.hpp>
#include <impl/serializable.hpp>

namespace vt::codec
{
	struct rle_mask : public impl::serializable
	{
		std::vector<uint32_t> runs;
		bool start_value{};
		utils::vec2<int> size{};

		static rle_mask encode(const uint8_t* mask_data, const utils::vec2<int>& size);
		static std::vector<uint8_t> decode(const rle_mask& mask_data);

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
}
