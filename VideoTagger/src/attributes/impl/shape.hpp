#pragma once
#include <events/event_source.hpp>
#include <impl/serializable.hpp>
#include <utils/vec.hpp>
#include <optional>
#include <imgui.h>
#include <core/types.hpp>

namespace vt::impl
{
	class shape : public serializable
	{
	public:
		shape() = default;
		virtual ~shape() = default;

	public:
		virtual void set_target(event_source source, video_id_t video_id) = 0;

		virtual utils::vec2<int>* closest_point(utils::vec2<int> point, float max_distance = std::numeric_limits<float>::infinity()) = 0;

		virtual std::vector<utils::vec2<int>*> get_all_points() = 0;

		virtual bool contains(utils::vec2<int> point) const = 0;

		virtual void render_shape(utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color) = 0;
		virtual void render_points(float radius, utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color) = 0;
		virtual void render(utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius);

		virtual bool render_data(event_source source, video_id_t video_id, utils::vec2<int> shape_space) = 0;
	};
}

namespace vt::math
{
	template<typename shape_type>
	shape_type shape_lerp(const shape_type& start, const shape_type& end, float alpha) = delete;
}
