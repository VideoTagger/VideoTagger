#pragma once
#include <vector>
#include <cstdint>
#include <utils/vec.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	class circle_shape : public impl::shape
	{
	public:
		circle_shape() = default;
		circle_shape(const utils::vec2<uint32_t>& pos, uint32_t radius);

	public:
		utils::vec2<uint32_t> pos;
		uint32_t radius = 1;

	public:
		bool operator==(const circle_shape& other) const;

		virtual void set_target(event_source source) override;

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
}
