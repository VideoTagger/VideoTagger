#pragma once
#include <vector>
#include <cstdint>
#include <utils/vec.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	class points_shape : public impl::shape
	{
	public:
		points_shape() = default;
		points_shape(const std::vector<utils::vec2<uint32_t>>& points);

	public:
		std::vector<utils::vec2<uint32_t>> points;

	public:
		bool operator==(const points_shape& other) const;

		virtual void set_target(event_source source) override;

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
}
