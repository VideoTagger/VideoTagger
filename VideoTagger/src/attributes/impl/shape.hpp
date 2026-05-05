#pragma once
#include <events/event_source.hpp>
#include <impl/serializable.hpp>
#include <utils/vec.hpp>
#include <optional>
#include <imgui.h>

namespace vt::impl
{
	class shape : public serializable
	{
	public:
		shape() = default;
		virtual ~shape() = default;

	public:
		virtual void set_target(event_source source) = 0;

		virtual const utils::vec2<uint32_t>* closest_point(utils::vec2<uint32_t> point, float max_distance = std::numeric_limits<float>::infinity()) const = 0;
		utils::vec2<uint32_t>* closest_point(utils::vec2<uint32_t> point, float max_distance = std::numeric_limits<float>::infinity());

		virtual bool contains(utils::vec2<uint32_t> point) const = 0;

		virtual void render_shape(utils::vec2<uint32_t> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color) = 0;
		virtual void render_points(float radius, utils::vec2<uint32_t> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color) = 0;
		virtual void render(utils::vec2<uint32_t> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius);
	};
}

namespace vt::math
{
	template<typename shape_type>
	shape_type shape_lerp(const shape_type& start, const shape_type& end, float alpha) = delete;
}
