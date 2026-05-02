#include "circle_shape.hpp"
#include <core/app_context.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <utils/intersection.hpp>
#include <core/debug.hpp>

namespace vt
{
	circle_shape::circle_shape(const utils::vec2<uint32_t>& pos, uint32_t radius) : pos{ pos }, radius{ radius } {}

	bool circle_shape::operator==(const circle_shape& other) const
	{
		return radius == other.radius and pos == other.pos;
	}

	void circle_shape::set_target(event_source source)
	{
		std::vector<utils::vec2<uint32_t>*> targets{ { &pos } };
		ctx_.dispatch_event<gizmo_set_targets_event>(source, targets);
	}

	bool circle_shape::contains(utils::vec2<uint32_t> point) const
	{
		return utils::intersection::is_in_circle(ImVec2(point[0], point[1]), ImVec2(pos[0], pos[1]), radius);
	}

	const utils::vec2<uint32_t>* circle_shape::closest_point(utils::vec2<uint32_t> point, float max_distance) const
	{
		if (utils::vec2<uint32_t>::distance(pos, point) > max_distance) return nullptr;
		return &pos;
	}

	void circle_shape::render_shape(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		auto scaled_pos = impl::shape::scale_point(pos, shape_space, draw_rect);
		auto scaled_radius = impl::shape::scale_point({ radius, radius }, shape_space, ImRect{ {}, draw_rect.GetSize() });

		draw_list->AddEllipseFilled(scaled_pos, scaled_radius, fill_color);
		draw_list->AddEllipse(scaled_pos, scaled_radius, outline_color);
	}

	void circle_shape::render_points(float radius, utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		auto scaled_pos = impl::shape::scale_point(pos, shape_space, draw_rect);

		draw_list->AddCircleFilled(scaled_pos, radius, fill_color);
	}

	[[nodiscard]] nlohmann::ordered_json circle_shape::serialize() const
	{
		nlohmann::ordered_json json;
		json["position"] = pos;
		json["radius"] = radius;
		return json;
	}

	void circle_shape::deserialize(const nlohmann::ordered_json& json)
	{
		if (!json.contains("position"))
		{
			debug::error("Invalid JSON: missing 'position' field");
			return;
		}

		if (!json.contains("radius") or !json["radius"].is_number_integer())
		{
			debug::error("Invalid JSON: missing or invalid 'radius' field");
			return;
		}

		pos = json["position"];
		radius = json["radius"];
	}
}
