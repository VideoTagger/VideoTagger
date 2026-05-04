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
		//TODO: make this more generic and move somewhere into utils
		static float scale_value(uint32_t value, uint32_t max_value, float draw_start, float draw_end);
		static ImVec2 scale_point(utils::vec2<uint32_t> point, utils::vec2<uint32_t> point_space, ImRect draw_rect);

		virtual void set_target(event_source source) = 0;

		virtual const utils::vec2<uint32_t>* closest_point(utils::vec2<uint32_t> point, float max_distance = std::numeric_limits<float>::infinity()) const = 0;
		utils::vec2<uint32_t>* closest_point(utils::vec2<uint32_t> point, float max_distance = std::numeric_limits<float>::infinity());

		virtual bool contains(utils::vec2<uint32_t> point) const = 0;

		virtual void render_shape(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color) = 0;
		virtual void render_points(float radius, utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color) = 0;
		void render(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color, std::optional<float> point_radius);
	};
}

namespace vt::math
{
	template<typename shape_type>
	shape_type shape_lerp(const shape_type& start, const shape_type& end, float alpha) = delete;
}
