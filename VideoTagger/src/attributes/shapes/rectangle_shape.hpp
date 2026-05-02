#pragma once
#include <cstdint>
#include <utils/vec.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	class rectangle_shape : public impl::shape
	{
	public:
		rectangle_shape() = default;
		rectangle_shape(const utils::vec2<uint32_t>& start, const utils::vec2<uint32_t>& end);

	public:
		utils::vec2<uint32_t> start;
		utils::vec2<uint32_t> end;

	public:
		bool operator==(const rectangle_shape& other) const;

		virtual void set_target(event_source source) override;

		virtual bool contains(utils::vec2<uint32_t> point) const override;
		virtual const utils::vec2<uint32_t>* closest_point(utils::vec2<uint32_t> point, float max_distance = std::numeric_limits<float>::infinity()) const override;

		virtual void render_shape(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color) override;
		virtual void render_points(float radius, utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color) override;

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
}

namespace vt::math
{
	template<>
	inline rectangle_shape shape_lerp<rectangle_shape>(const rectangle_shape& start, const rectangle_shape& end, float alpha)
	{
		return rectangle_shape
		{
			math::lerp(start.start, end.start, alpha),
			math::lerp(start.end, end.end, alpha)
		};
	}
}
